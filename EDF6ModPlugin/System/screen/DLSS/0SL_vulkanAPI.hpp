#pragma once
#include <d3d11.h>
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
// dlss
#include "lib/nvsdk_ngx_vk.h"
#include "lib/nvsdk_ngx_helpers_vk.h"
#include "lib/nvsdk_ngx_helpers_dlssg_vk.h"

class IDXGIVkInteropDevice;

// from https://github.com/doitsujin/dxvk/blob/70d7508c01201ed3d4bfb33da42ba834eafe3857/src/dxgi/dxgi_interfaces.h#L262
static const GUID IID_IDXGIVkInteropSurface = { 0x5546cf8c, 0x77e7, 0x4341, { 0xb0, 0x5d, 0x8d, 0x4d, 0x50, 0x00, 0xe7, 0x7d } };
class IDXGIVkInteropSurface : public IUnknown {
public:
	/**
	 * \brief Retrieves device interop interfaceSlots
	 *
	 * Queries the device that owns the surface for
	 * the \ref IDXGIVkInteropDevice interface.
	 * \param [out] ppDevice The device interface
	 * \returns \c S_OK on success
	 */
	virtual HRESULT STDMETHODCALLTYPE GetDevice(
		IDXGIVkInteropDevice** ppDevice) = 0;

	/**
	 * \brief Retrieves Vulkan image info
	 *
	 * Retrieves both the image handle as well as the image's
	 * properties. Any of the given pointers may be \c nullptr.
	 *
	 * If \c pInfo is not \c nullptr, the following rules apply:
	 * - \c pInfo->sType \e must be \c VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO
	 * - \c pInfo->pNext \e must be \c nullptr or point to a supported
	 *   extension-specific structure (currently none)
	 * - \c pInfo->queueFamilyIndexCount must be the length of the
	 *   \c pInfo->pQueueFamilyIndices array, in \c uint32_t units.
	 * - \c pInfo->pQueueFamilyIndices must point to a pre-allocated
	 *   array of \c uint32_t of size \c pInfo->pQueueFamilyIndices.
	 *
	 * \note As of now, the sharing mode will always be
	 *       \c VK_SHARING_MODE_EXCLUSIVE and no queue
	 *       family indices will be written to the array.
	 *
	 * After the call, the structure pointed to by \c pInfo can
	 * be used to create an image with identical properties.
	 *
	 * If \c pLayout is not \c nullptr, it will receive the
	 * layout that the image will be in after flushing any
	 * outstanding commands on the device.
	 * \param [out] pHandle The image handle
	 * \param [out] pLayout Image layout
	 * \param [out] pInfo Image properties
	 * \returns \c S_OK on success, or \c E_INVALIDARG
	 */
	virtual HRESULT STDMETHODCALLTYPE GetVulkanImageInfo(
		VkImage* pHandle,
		VkImageLayout* pLayout,
		VkImageCreateInfo* pInfo) = 0;
};

// from https://github.com/doitsujin/dxvk/blob/70d7508c01201ed3d4bfb33da42ba834eafe3857/src/dxgi/dxgi_interfaces.h#L318
// is DXVK 3.1
static const GUID IID_IDXGIVkInteropDevice = { 0xe2ef5fa5, 0xdc21, 0x4af7, { 0x90, 0xc4, 0xf6, 0x7e, 0xf6, 0xa0, 0x93, 0x23 } };
//MIDL_INTERFACE("e2ef5fa5-dc21-4af7-90c4-f67ef6a09323")
class IDXGIVkInteropDevice : public IUnknown {
public:
	/**
	 * \brief Queries Vulkan handles used by DXVK
	 *
	 * \param [out] pInstance The Vulkan instance
	 * \param [out] pPhysDev The physical device
	 * \param [out] pDevide The device handle
	 */
	virtual void STDMETHODCALLTYPE GetVulkanHandles(
		VkInstance* pInstance,
		VkPhysicalDevice* pPhysDev,
		VkDevice* pDevice) = 0;

	/**
	 * \brief Queries the rendering queue used by DXVK
	 *
	 * \param [out] pQueue The Vulkan queue handle
	 * \param [out] pQueueFamilyIndex Queue family index
	 */
	virtual void STDMETHODCALLTYPE GetSubmissionQueue(
		VkQueue* pQueue,
		uint32_t* pQueueFamilyIndex) = 0;

	/**
	 * \brief Transitions a surface to a given layout
	 *
	 * Executes an explicit image layout transition on the
	 * D3D device. Note that the image subresources \e must
	 * be transitioned back to its original layout before
	 * using it again from D3D11.
	 * \param [in] pSurface The image to transform
	 * \param [in] pSubresources Subresources to transform
	 * \param [in] OldLayout Current image layout
	 * \param [in] NewLayout Desired image layout
	 */
	virtual void STDMETHODCALLTYPE TransitionSurfaceLayout(
		IDXGIVkInteropSurface* pSurface,
		const VkImageSubresourceRange* pSubresources,
		VkImageLayout             OldLayout,
		VkImageLayout             NewLayout) = 0;

	/**
	 * \brief Flushes outstanding D3D rendering commands
	 *
	 * Must be called before submitting Vulkan commands
	 * to the rendering queue if those commands use the
	 * backing resource of a D3D11 object.
	 */
	virtual void STDMETHODCALLTYPE FlushRenderingCommands() = 0;

	/**
	 * \brief Locks submission queue
	 *
	 * Should be called immediately before submitting
	 * Vulkan commands to the rendering queue in order
	 * to prevent DXVK from using the queue.
	 *
	 * While the submission queue is locked, no D3D11
	 * methods must be called from the locking thread,
	 * or otherwise a deadlock might occur.
	 */
	virtual void STDMETHODCALLTYPE LockSubmissionQueue() = 0;

	/**
	 * \brief Releases submission queue
	 *
	 * Should be called immediately after submitting
	 * Vulkan commands to the rendering queue in order
	 * to allow DXVK to submit new commands.
	 */
	virtual void STDMETHODCALLTYPE ReleaseSubmissionQueue() = 0;
};

namespace D3D{
	class slVulkanAPI_t {
	public:
		// function ======================================
		PFN_vkCreateCommandPool vkCreateCommandPool = nullptr;
		PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers = nullptr;
		PFN_vkFreeCommandBuffers vkFreeCommandBuffers = nullptr;
		PFN_vkDestroyCommandPool vkDestroyCommandPool = nullptr;
		PFN_vkBeginCommandBuffer vkBeginCommandBuffer = nullptr;
		PFN_vkEndCommandBuffer vkEndCommandBuffer = nullptr;
		PFN_vkResetCommandBuffer vkResetCommandBuffer = nullptr;

		PFN_vkQueueSubmit vkQueueSubmit = nullptr;
		PFN_vkQueueWaitIdle vkQueueWaitIdle= nullptr;

		PFN_vkCreateFence vkCreateFence = nullptr;
		PFN_vkDestroyFence vkDestroyFence = nullptr;
		PFN_vkWaitForFences vkWaitForFences = nullptr;
		PFN_vkResetFences vkResetFences = nullptr;

		PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements = nullptr;
		PFN_vkCreateImageView vkCreateImageView = nullptr;
		PFN_vkDestroyImageView vkDestroyImageView = nullptr;
		// function end =====================================

		HMODULE vulkanModule = nullptr;

		IDXGIVkInteropDevice* m_vkInterop;
		VkDevice m_vkDevice; VkInstance m_vkInstance; VkPhysicalDevice m_vkPhysDevice;
		VkQueue m_vkQueue; VkCommandPool m_vkCMDPool; VkCommandBuffer m_vkCMDlist;
		VkFence m_fence; uint32_t m_QueueFamilyIndex;
		void Initialization(){
			vulkanModule = GetModuleHandleW(L"vulkan-1.dll");
			if (!vulkanModule) return;

			vkCreateCommandPool = (PFN_vkCreateCommandPool)GetProcAddress(vulkanModule, "vkCreateCommandPool");
			vkAllocateCommandBuffers = (PFN_vkAllocateCommandBuffers)GetProcAddress(vulkanModule, "vkAllocateCommandBuffers");
			vkFreeCommandBuffers = (PFN_vkFreeCommandBuffers)GetProcAddress(vulkanModule, "vkFreeCommandBuffers");
			vkDestroyCommandPool = (PFN_vkDestroyCommandPool)GetProcAddress(vulkanModule, "vkDestroyCommandPool");
			vkBeginCommandBuffer = (PFN_vkBeginCommandBuffer)GetProcAddress(vulkanModule, "vkBeginCommandBuffer");
			vkEndCommandBuffer = (PFN_vkEndCommandBuffer)GetProcAddress(vulkanModule, "vkEndCommandBuffer");
			vkResetCommandBuffer = (PFN_vkResetCommandBuffer)GetProcAddress(vulkanModule, "vkResetCommandBuffer");

			vkQueueSubmit = (PFN_vkQueueSubmit)GetProcAddress(vulkanModule, "vkQueueSubmit");
			vkQueueWaitIdle = (PFN_vkQueueWaitIdle)GetProcAddress(vulkanModule, "vkQueueWaitIdle");

			vkCreateFence = (PFN_vkCreateFence)GetProcAddress(vulkanModule, "vkCreateFence");
			vkDestroyFence = (PFN_vkDestroyFence)GetProcAddress(vulkanModule, "vkDestroyFence");
			vkWaitForFences = (PFN_vkWaitForFences)GetProcAddress(vulkanModule, "vkWaitForFences");
			vkResetFences = (PFN_vkResetFences)GetProcAddress(vulkanModule, "vkResetFences");

			vkGetImageMemoryRequirements = (PFN_vkGetImageMemoryRequirements)GetProcAddress(vulkanModule, "vkGetImageMemoryRequirements");
			vkCreateImageView = (PFN_vkCreateImageView)GetProcAddress(vulkanModule, "vkCreateImageView");
			vkDestroyImageView = (PFN_vkDestroyImageView)GetProcAddress(vulkanModule, "vkDestroyImageView");
		}

		void CreateFence() {
			VkCommandPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			poolInfo.queueFamilyIndex = m_QueueFamilyIndex;
			vkCreateCommandPool(m_vkDevice, &poolInfo, nullptr, &m_vkCMDPool);

			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.commandPool = m_vkCMDPool;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandBufferCount = 1;
			vkAllocateCommandBuffers(m_vkDevice, &allocInfo, &m_vkCMDlist);

			VkFenceCreateInfo fenceInfo{};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			vkCreateFence(m_vkDevice, &fenceInfo, nullptr, &m_fence);
		}

		void VK_Enter(int needQueueWait = 0) {
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			m_vkInterop->FlushRenderingCommands();
			m_vkInterop->LockSubmissionQueue();

			if (needQueueWait) {
				vkQueueWaitIdle(m_vkQueue);
			}

			vkResetCommandBuffer(m_vkCMDlist, 0);
			vkBeginCommandBuffer(m_vkCMDlist, &beginInfo);
		}

		void VK_Leave(VkFence pFence) {
			vkEndCommandBuffer(m_vkCMDlist);

			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &m_vkCMDlist;
			vkQueueSubmit(m_vkQueue, 1, &submitInfo, pFence);

			m_vkInterop->ReleaseSubmissionQueue();

			if (pFence == VK_NULL_HANDLE) return;

			vkWaitForFences(m_vkDevice, 1, &pFence, VK_TRUE, UINT64_MAX);
			vkResetFences(m_vkDevice, 1, &pFence);
		}
		// end
	};
	typedef slVulkanAPI_t* PslVulkanAPI;
}


