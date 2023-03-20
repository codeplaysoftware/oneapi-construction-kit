# Copyright (C) Codeplay Software Limited. All Rights Reserved.

from __future__ import unicode_literals
import os
import sys
import threading
import traceback
import datetime
import re
from execution import TestQueue, Worker
from execution import get_cpu_count, CV_TIMEOUT
from test_info import TestResults
from profile import Profile
from ui import TestUI
from clik import ClikProfile

try:
    FileNotFoundError
except NameError:
    FileNotFoundError = IOError


def escape_byte(m):
    """
    Helper function that replaces an invalid character with an hex escape.
    :param m: Match object as returned from a 're' function.
    :return: Replacement bytes.
    """
    byte = m.group(0)[0]
    return ("\\x%02x" % byte).encode("ascii")


class TestRunner(object):
    def __init__(self):
        self.profile = None
        self.args = None
        self.tests = None
        self.num_tests = 0
        self.queue = None
        self.results = None
        self.workers = []
        self.workers_state = []
        self.num_workers = 0
        self.running_workers = 0
        self.live_tests = set()
        self.aborted = False
        self.lock = threading.RLock()
        self.cond = threading.Condition(self.lock)
        self.ui = None
        self.log_file = None
        self.log = None

    def load_profile(self):
        """ Initialize the test runner by loading the profile and parsing
        options. """
        new_argv = self.preprocess_args(sys.argv[1:])
        self.profile = ClikProfile()
        self.profile.runner = self
        self.args = self.profile.parse_options(new_argv)
        self.workers_state = self.profile.init_workers_state(self.args.jobs)
        # Set up the "UI".
        self.ui = TestUI()
        if self.args.no_color:
            self.ui.use_color = False

    def preprocess_args(self, args):
        """
        Pre-process command-line arguments, expanding response files.
        :param args: List of arguments to pre-process.
        :return: List of pre-processed arguments.
        """
        new_args = []
        for arg in args:
            if not arg.startswith("@"):
                new_args.append(arg)
                continue
            response_file = arg[1:]
            if not os.path.exists(response_file):
                raise FileNotFoundError("Response file not found: %s" %
                                        response_file)
            with open(response_file, "r") as f:
                for line in f:
                    new_args.append(line.strip())
        return new_args

    def print_exception(self, exc):
        if self.profile and self.profile.args.verbose:
            traceback.print_exc()
        else:
            sys.stderr.write("error: %s\n" % exc)

    def request_worker_state(self, worker_id):
        """
        Assumes that the worker state info will be provided for either all of
        the workers or none of them.
        """
        with self.lock:
            if not self.workers_state:
                return None
            return self.workers_state[worker_id]

    def execute(self):
        # Use profile to find list of tests
        test_source = self.args.test_source  # CSV
        self.tests = self.profile.load_tests(test_source)
        if self.args.repeat > 1:
          self.tests *= self.args.repeat
        self.num_tests = len(self.tests)

        # Determine how many workers should be used.
        if self.args.jobs:
            if self.args.jobs < 0:
                raise Exception("Invalid number of workers: %d" %
                                self.args.jobs)
            self.num_workers = self.args.jobs
        else:
            self.num_workers = get_cpu_count()
        # Spread tests between workers, using a queue.
        self.results = TestResults(self.tests)
        self.results.start_time = datetime.datetime.now()
        if self.args.log_file:
            self.log_file = open(self.args.log_file, "w")
            self.log = TestUI(self.log_file)
            self.log.print_start_banner(self.results, self.num_workers)
        self.ui.print_start_banner(self.results, self.num_workers)

        self.queue = TestQueue(self.tests.tests)

        with self.lock:
            self.running_workers = self.num_workers
            self.results.start()
        for i in range(0, self.num_workers):
            worker = Worker(i, self)
            self.workers.append(worker)
            worker.start()
        # Wait for workers to finish.
        self.wait_for_workers()
        return self.process_results()

    def wait_for_workers(self):
        """ Wait for workers to finish executing tests. This also handles
        the task of aborting tests that exceed their deadline. """
        sleep_interval = CV_TIMEOUT
        with self.lock:
            if self.profile.timeout:
                sleep_interval = 1
            while not self.results.finished and (self.running_workers > 0):
                try:
                    self.cond.wait(sleep_interval)
                except KeyboardInterrupt:
                    self.aborted = True
                    break
                self.enforce_deadlines()
            self.results.stop()
            self.queue.close()

            while self.running_workers > 0:
                self.cond.wait(CV_TIMEOUT)
            self.results.finish(self.profile)

    def enforce_deadlines(self):
        """ Abort tests that have exceeded their deadline. """
        if not self.profile.timeout:
            return
        current_time = datetime.datetime.now()
        for schedule in self.live_tests:
            if schedule.deadline and (schedule.deadline <= current_time):
                for process in schedule.processes:
                    try:
                        process.terminate()
                    except Exception:
                        pass
                schedule.aborted = True

    def process_results(self):
        """
        Print test results to the console and create result files.
        :return: 1 if there were test failures or 0 for no failure.
        """
        # Print the results.
        self.ui.print_results(self.results)
        if self.log:
            self.log.print_results(self.results)
        # Write the list of failures.
        if self.args.fail_file:
            with open(self.args.fail_file, "w") as f:
                for run in self.results.fail_list:
                    f.write(run.test.name + "\n")
        # Close the log file.
        if self.log_file:
            self.log_file.close()
            self.log_file = None
        # Generate the exit code. By default we return 0 at exit even when
        # there are failing tests so that CI jobs can report unstable results
        # based on the XML output rather than solely on the exit code.
        exit_code = 0
        if self.aborted:
            # The run was externally aborted.
            exit_code = 1
        if self.results.num_fails:
            if self.args.strict:
                # Any test failed and the strict flag was set.
                exit_code = 1
            if self.results.num_tests == self.results.num_fails:
                # All tests failed.
                exit_code = 1
        return exit_code

    def process_output(self, run):
        """
        Decode the test process' output and process it so that it can be
        printed and logged safely.
        :param run: Test run whose output to process.
        """
        if run.output:
            # Escape non-printable and non-ASCII characters from the test
            # output.
            escaped_output = re.sub(b"[\x00-\x08\x0b\x0c\x0e-\x1f\x80-\xff]",
                                    escape_byte, run.output)
            run.output = escaped_output.decode("ascii")

    def test_started(self, run):
        """
        This function is called when a test has started executing.
        :param run: Test which has started executing.
        """
        with self.lock:
            run.schedule.start_time = datetime.datetime.now()
            if self.profile.timeout:
                timeout = datetime.timedelta(0, self.profile.timeout)
                run.schedule.deadline = run.schedule.start_time + timeout
            self.live_tests.add(run.schedule)

    def test_created_process(self, schedule, process):
        """
        This function is called when a test has spawned a process.
        :param schedule: Schedule of the test which has spawned the process.
        :param process: Process that was spawned.
        """
        with self.lock:
            schedule.processes.append(process)

    def dequeue_test(self):
        """
        Pop a test which is eligible to execute.
        :return: Test for worker thread to run.
        """
        with self.lock:
            test = self.queue.dequeue()
            if test:
                return test
        # Causes Worker thread to terminate
        raise EOFError("No eligible tests")

    def test_finished(self, run):
        """
        This function is called when a test has been executed.
        :param run: Test which has been executed.
        """
        with self.lock:
            run.schedule.end_time = datetime.datetime.now()
            self.live_tests.remove(run.schedule)
            self.results.add_run(run)
            self.ui.print_test_status(run, self.results)
            if self.log:
                self.log.print_test_status(run, self.results)
            self.process_output(run)
            if self.args.verbose or run.status not in ["PASS", "SKIP"]:
                self.ui.print_test_output(run, self.results)
            if self.log:
                self.log.print_test_output(run, self.results)
            self.cond.notify_all()

    def worker_started(self, worker):
        """
        This function is called when a worker has started executing tests.
        :param worker: Worker which started executing tests.
        """
        pass

    def worker_stopped(self, worker, exc=None):
        """
        This function is called when a worker stops executing tests.
        :param worker: Worker which stopped executing tests.
        :param exc: Optional exception that resulted in the stop.
        """
        aborted = False
        if exc:
            self.ui.print_worker_exception(worker, exc)
            aborted = True
        with self.lock:
            self.running_workers -= 1
            if aborted:
                self.aborted = True
            self.cond.notify_all()