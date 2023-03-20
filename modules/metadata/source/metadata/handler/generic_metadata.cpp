// Copyright (C) Codeplay Software Limited. All Rights Reserved.

#include <metadata/detail/utils.h>
#include <metadata/handler/generic_metadata.h>

namespace handler {

GenericMetadata::GenericMetadata(std::string kernel_name,
                                 std::string source_name,
                                 uint64_t local_memory_usage)
    : kernel_name(std::move(kernel_name)),
      source_name(std::move(source_name)),
      local_memory_usage(local_memory_usage) {}

GenericMetadata::GenericMetadata(
    std::string kernel_name, std::string source_name,
    uint64_t local_memory_usage,
    FixedOrScalableQuantity<uint32_t> sub_group_size)
    : kernel_name(std::move(kernel_name)),
      source_name(std::move(source_name)),
      local_memory_usage(local_memory_usage),
      sub_group_size(sub_group_size) {}

GenericMetadataHandler::~GenericMetadataHandler() {
  if (ctx) {
    md_release_ctx(ctx);
  }
  if (data) {
    if (hooks && hooks->deallocate) {
      hooks->deallocate(data, userdata);
    } else {
      std::free(data);
    }
  }
}

bool GenericMetadataHandler::init(md_hooks *hooks, void *userdata) {
  this->hooks = hooks;
  this->userdata = userdata;
  ctx = md_init(hooks, userdata);
  if (!ctx) {
    return false;
  }
  md_stack generic_stack = md_get_block(ctx, GENERIC_MD_BLOCK_NAME);
  if (!generic_stack) {
    generic_stack = md_create_block(ctx, GENERIC_MD_BLOCK_NAME);
    if (!generic_stack) {
      return false;
    }
  }

  if (hooks->map) {
    const int err = md_loadf(generic_stack, "s", &data_len, &data);
    if (MD_CHECK_ERR(err)) {
      return false;
    }
  }
  return true;
}

bool GenericMetadataHandler::finalize() {
  md_stack generic_stack = md_get_block(ctx, GENERIC_MD_BLOCK_NAME);
  if (!generic_stack) {
    return false;
  }
  int err = md_finalize_block(generic_stack);
  if (MD_CHECK_ERR(err)) {
    return false;
  }
  err = md_finalize_ctx(ctx);
  if (MD_CHECK_ERR(err)) {
    return false;
  }
  return true;
}

bool GenericMetadataHandler::read(GenericMetadata &md) {
  if (offset >= data_len) {
    return false;
  }

  std::string kernel_name(data + offset);
  offset += kernel_name.size() + 1;

  std::string source_name(data + offset);
  offset += source_name.size() + 1;

  uint64_t local_memory_used = md::utils::read_value<uint64_t>(
      (uint8_t *)data + offset, md_get_endianness(ctx));
  offset += sizeof(uint64_t);

  // We only use the low 4 bytes of this value, even though it's encoded as 8.
  uint32_t sub_group_size_fixed = md::utils::read_value<uint64_t>(
      (uint8_t *)data + offset, md_get_endianness(ctx));
  offset += sizeof(uint64_t);
  bool sub_group_size_is_scalable =
      md::utils::read_value<uint64_t>((uint8_t *)data + offset,
                                      md_get_endianness(ctx)) == 1;
  offset += sizeof(uint64_t);

  md.kernel_name = std::move(kernel_name);
  md.source_name = std::move(source_name);
  md.local_memory_usage = local_memory_used;
  md.sub_group_size = FixedOrScalableQuantity<uint32_t>(
      sub_group_size_fixed, sub_group_size_is_scalable);
  return true;
}

bool GenericMetadataHandler::write(const GenericMetadata &md) {
  md_stack generic_stack = md_get_block(ctx, GENERIC_MD_BLOCK_NAME);
  if (!generic_stack) {
    return false;
  }
  int err = md_push_zstr(generic_stack, md.kernel_name.c_str());
  if (MD_CHECK_ERR(err)) {
    return false;
  }
  err = md_push_zstr(generic_stack, md.source_name.c_str());
  if (MD_CHECK_ERR(err)) {
    return false;
  }
  err = md_push_uint(generic_stack, md.local_memory_usage);
  if (MD_CHECK_ERR(err)) {
    return false;
  }
  err = md_push_uint(generic_stack, md.sub_group_size.getKnownMinValue());
  if (MD_CHECK_ERR(err)) {
    return false;
  }
  err = md_push_uint(generic_stack, md.sub_group_size.isScalable() ? 1 : 0);
  if (MD_CHECK_ERR(err)) {
    return false;
  }
  return true;
}

}  // namespace handler