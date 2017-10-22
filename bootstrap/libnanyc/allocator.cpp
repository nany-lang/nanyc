#include <nanyc/allocator.h>
#include "libnanyc.h"
#include "libnanyc-config.h"
#include <yuni/core/string.h>

namespace {

void import_allocator_cf(nyallocator_t* allocator, const nyallocator_opts_t* opts) {
	allocator->userdata = opts->userdata;
	allocator->allocate = opts->allocate;
	allocator->deallocate = opts->deallocate;
	allocator->reallocate = opts->reallocate;
	allocator->transfer_ownership_from_malloc = opts->transfer_ownership_from_malloc;
	allocator->on_release = opts->on_release;
	allocator->on_not_enough_memory = opts->on_not_enough_memory;
}

void* report_not_enough_memory_for_allocation(nyallocator_t* allocator, size_t size) {
	if (allocator->on_not_enough_memory) {
		yuni::ShortString128 msg;
		msg << "not enough memory to allocate " << size << " bytes";
		allocator->on_not_enough_memory(allocator, msg.c_str(), msg.size());
	}
	return nullptr;
}

void* nanyc_sys_allocate(nyallocator_t* allocator, size_t size) {
	void* p = malloc(size);
	if (likely(p))
		return p;
	return report_not_enough_memory_for_allocation(allocator, size);
}

void* nanyc_sys_reallocate(nyallocator_t* allocator, void* ptr, size_t /*old_size*/, size_t new_size) {
	void* p = realloc(ptr, new_size);
	if (likely(p))
		return p;
	return report_not_enough_memory_for_allocation(allocator, new_size);
}

void nanyc_sys_deallocate(nyallocator_t*, void* ptr, size_t /*size*/) {
	free(ptr);
}

void* nanyc_sys_transfer_ownership_from_malloc(nyallocator_t* allocator, void* ptr, size_t size) {
	size += ny::config::extraObjectSize;
	void* p = realloc(ptr, size);
	if (likely(p))
		return p;
	return report_not_enough_memory_for_allocation(allocator, size);
}

} // namespace

nyallocator_t* nyallocator_make(const nyallocator_opts_t* opts) {
	auto* allocator = (nyallocator_t*)::malloc(sizeof(nyallocator_t));
	if (allocator)
		import_allocator_cf(allocator, opts);
	return allocator;
}

void nyallocator_init(nyallocator_t* allocator, const nyallocator_opts_t* opts) {
	import_allocator_cf(allocator, opts);
}

void nyallocator_init_from_malloc(nyallocator_t* allocator) {
	allocator->userdata = nullptr;
	allocator->allocate = &nanyc_sys_allocate;
	allocator->deallocate = &nanyc_sys_deallocate;
	allocator->reallocate = &nanyc_sys_reallocate;
	allocator->transfer_ownership_from_malloc = &nanyc_sys_transfer_ownership_from_malloc;
	allocator->on_release = nullptr;
	allocator->on_not_enough_memory = nullptr;
}

nyallocator_t* nyallocator_make_from_malloc() {
	auto* allocator = (nyallocator_t*)::malloc(sizeof(nyallocator_t));
	if (allocator)
		nyallocator_init_from_malloc(allocator);
	return allocator;
}

void nyallocator_dispose(nyallocator_t* allocator) {
	if (allocator) {
		if (allocator->on_release)
			allocator->on_release(allocator);
		free(allocator);
	}
}
