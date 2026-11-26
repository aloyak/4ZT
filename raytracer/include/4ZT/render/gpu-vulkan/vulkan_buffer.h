#pragma once

#include "4ZT/render/gpu-vulkan/vulkan_deps.h"

#include <stdexcept>
#include <cstring>

namespace FourZT {

class GpuBuffer {
public:
    // hostVisible: CPU can read/write it directly (fine for the test; later, device-local + staging)
    GpuBuffer(VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usage, bool hostVisible)
        : m_allocator(allocator), m_size(size) {
        VkBufferCreateInfo bi{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        bi.size  = size;
        bi.usage = usage;

        VmaAllocationCreateInfo ai{};
        ai.usage = VMA_MEMORY_USAGE_AUTO;
        if (hostVisible)
            ai.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

        VmaAllocationInfo info{};
        if (vmaCreateBuffer(allocator, &bi, &ai, &m_buffer, &m_alloc, &info) != VK_SUCCESS)
            throw std::runtime_error("4ZT: vmaCreateBuffer failed");
        m_mapped = info.pMappedData;
    }
    ~GpuBuffer() { if (m_buffer) vmaDestroyBuffer(m_allocator, m_buffer, m_alloc); }

    GpuBuffer(const GpuBuffer&) = delete;
    GpuBuffer& operator=(const GpuBuffer&) = delete;

    // uploads data to the buffer (must be hostVisible)
    void Upload(const void* src, VkDeviceSize bytes) const {
        if (!m_mapped || bytes > m_size) throw std::runtime_error("4ZT: GpuBuffer::Upload misuse");
        std::memcpy(m_mapped, src, size_t(bytes));
        vmaFlushAllocation(m_allocator, m_alloc, 0, bytes); // no-op on coherent memory
    }

    VkBuffer       handle()     const { return m_buffer; }
    VmaAllocation  allocation() const { return m_alloc; }
    void*          mapped()     const { return m_mapped; }
    VkDeviceSize   size()       const { return m_size; }

private:
    VmaAllocator  m_allocator = VK_NULL_HANDLE;
    VkBuffer      m_buffer = VK_NULL_HANDLE;
    VmaAllocation m_alloc = VK_NULL_HANDLE;
    void*         m_mapped = nullptr; // pointer to mapped memory (if hostVisible)
    VkDeviceSize  m_size = 0;
};

} // 4zt