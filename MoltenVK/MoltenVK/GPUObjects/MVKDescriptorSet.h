/*
 * MVKDescriptorSet.h
 *
 * Copyright (c) 2015-2020 The Brenwill Workshop Ltd. (http://www.brenwill.com)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "MVKDescriptorBinding.h"
#include "MVKVector.h"
#include <unordered_set>
#include <unordered_map>

class MVKDescriptorPool;
class MVKPipelineLayout;
class MVKCommandEncoder;


#pragma mark -
#pragma mark MVKDescriptorSetLayout

/** Represents a Vulkan descriptor set layout. */
class MVKDescriptorSetLayout : public MVKVulkanAPIDeviceObject {

public:

	/** Returns the Vulkan type of this object. */
	VkObjectType getVkObjectType() override { return VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT; }

	/** Returns the debug report object type of this object. */
	VkDebugReportObjectTypeEXT getVkDebugReportObjectType() override { return VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT; }

	/** Encodes this descriptor set layout and the specified descriptor set on the specified command encoder. */
    void bindDescriptorSet(MVKCommandEncoder* cmdEncoder,
                           MVKDescriptorSet* descSet,
                           MVKShaderResourceBinding& dslMTLRezIdxOffsets,
                           MVKVector<uint32_t>& dynamicOffsets,
                           uint32_t* pDynamicOffsetIndex);


	/** Encodes this descriptor set layout and the specified descriptor updates on the specified command encoder immediately. */
	void pushDescriptorSet(MVKCommandEncoder* cmdEncoder,
						   MVKVector<VkWriteDescriptorSet>& descriptorWrites,
						   MVKShaderResourceBinding& dslMTLRezIdxOffsets);


	/** Encodes this descriptor set layout and the updates from the given template on the specified command encoder immediately. */
	void pushDescriptorSet(MVKCommandEncoder* cmdEncoder,
						   MVKDescriptorUpdateTemplate* descUpdateTemplates,
						   const void* pData,
						   MVKShaderResourceBinding& dslMTLRezIdxOffsets);


	/** Populates the specified shader converter context, at the specified DSL index. */
	void populateShaderConverterContext(mvk::SPIRVToMSLConversionConfiguration& context,
                                        MVKShaderResourceBinding& dslMTLRezIdxOffsets,
                                        uint32_t dslIndex);

	/** Returns true if this layout is for push descriptors only. */
	bool isPushDescriptorLayout() const { return _isPushDescriptorLayout; }

	MVKDescriptorSetLayout(MVKDevice* device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo);

	~MVKDescriptorSetLayout();

protected:

	friend class MVKDescriptorSetLayoutBinding;
	friend class MVKPipelineLayout;
	friend class MVKDescriptorSet;
	friend class MVKDescriptorPool;

	void propogateDebugName() override {}
	void addDescriptorPool(MVKDescriptorPool* mvkDescPool) { _descriptorPools.insert(mvkDescPool); }
	void removeDescriptorPool(MVKDescriptorPool* mvkDescPool) { _descriptorPools.erase(mvkDescPool); }
	uint32_t getDescriptorIndex(uint32_t binding, uint32_t elementIndex);

	MVKVectorInline<MVKDescriptorSetLayoutBinding, 1> _bindings;
	std::unordered_map<uint32_t, uint32_t> _bindingToIndex;
	MVKShaderResourceBinding _mtlResourceCounts;
	std::unordered_set<MVKDescriptorPool*> _descriptorPools;
	bool _isPushDescriptorLayout : 1;
};


#pragma mark -
#pragma mark MVKDescriptorSet

/** Represents a Vulkan descriptor set. */
class MVKDescriptorSet : public MVKVulkanAPIDeviceObject, public MVKLinkableMixin<MVKDescriptorSet> {

public:

	/** Returns the Vulkan type of this object. */
	VkObjectType getVkObjectType() override { return VK_OBJECT_TYPE_DESCRIPTOR_SET; }

	/** Returns the debug report object type of this object. */
	VkDebugReportObjectTypeEXT getVkDebugReportObjectType() override { return VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT; }

	/** Updates the resource bindings in this instance from the specified content. */
	template<typename DescriptorAction>
	void writeDescriptorSets(const DescriptorAction* pDescriptorAction,
							 size_t stride,
							 const void* pData);

	/** 
	 * Reads the resource bindings defined in the specified content 
	 * from this instance into the specified collection of bindings.
	 */
	void readDescriptorSets(const VkCopyDescriptorSet* pDescriptorCopies,
							VkDescriptorType& descType,
							VkDescriptorImageInfo* pImageInfo,
							VkDescriptorBufferInfo* pBufferInfo,
							VkBufferView* pTexelBufferView,
							VkWriteDescriptorSetInlineUniformBlockEXT* pInlineUniformBlock);

	MVKDescriptorSet(MVKDevice* device) : MVKVulkanAPIDeviceObject(device) {}

protected:
	friend class MVKDescriptorSetLayoutBinding;
	friend class MVKDescriptorPool;

	void propogateDebugName() override {}
	void setLayout(MVKDescriptorSetLayout* layout);
	inline MVKDescriptorBinding* getDescriptor(uint32_t index) { return &_bindings[index]; }

	MVKDescriptorSetLayout* _pLayout = nullptr;
	MVKVectorInline<MVKDescriptorBinding, 1> _bindings;
};


#pragma mark -
#pragma mark MVKDescriptorPool

typedef MVKDeviceObjectPool<MVKDescriptorSet> MVKDescriptorSetPool;

/** Represents a Vulkan descriptor pool. */
class MVKDescriptorPool : public MVKVulkanAPIDeviceObject {

public:

	/** Returns the Vulkan type of this object. */
	VkObjectType getVkObjectType() override { return VK_OBJECT_TYPE_DESCRIPTOR_POOL; }

	/** Returns the debug report object type of this object. */
	VkDebugReportObjectTypeEXT getVkDebugReportObjectType() override { return VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT; }

	/** Allocates the specified number of descriptor sets. */
	VkResult allocateDescriptorSets(uint32_t count,
									const VkDescriptorSetLayout* pSetLayouts,
									VkDescriptorSet* pDescriptorSets);

	/** Free's up the specified descriptor set. */
	VkResult freeDescriptorSets(uint32_t count, const VkDescriptorSet* pDescriptorSets);

	/** Destoys all currently allocated descriptor sets. */
	VkResult reset(VkDescriptorPoolResetFlags flags);

	/** Removes the pool associated with a descriptor set layout. */
	void removeDescriptorSetPool(MVKDescriptorSetLayout* mvkDescSetLayout);

	MVKDescriptorPool(MVKDevice* device, const VkDescriptorPoolCreateInfo* pCreateInfo);

	~MVKDescriptorPool() override;

protected:
	void propogateDebugName() override {}
	MVKDescriptorSetPool* getDescriptorSetPool(MVKDescriptorSetLayout* mvkDescSetLayout);
	void returnDescriptorSet(MVKDescriptorSet* mvkDescSet);

	uint32_t _maxSets;
	std::unordered_set<MVKDescriptorSet*> _allocatedSets;
	std::unordered_map<MVKDescriptorSetLayout*, MVKDescriptorSetPool*> _descriptorSetPools;
};


#pragma mark -
#pragma mark MVKDescriptorUpdateTemplate

/** Represents a Vulkan descriptor update template. */
class MVKDescriptorUpdateTemplate : public MVKVulkanAPIDeviceObject {

public:

	/** Returns the Vulkan type of this object. */
	VkObjectType getVkObjectType() override { return VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE; }

	/** Returns the debug report object type of this object. */
	VkDebugReportObjectTypeEXT getVkDebugReportObjectType() override { return VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_EXT; }

	/** Get the nth update template entry. */
	const VkDescriptorUpdateTemplateEntryKHR* getEntry(uint32_t n) const;

	/** Get the total number of entries. */
	uint32_t getNumberOfEntries() const;

	/** Get the type of this template. */
	VkDescriptorUpdateTemplateTypeKHR getType() const;

	/** Constructs an instance for the specified device. */
	MVKDescriptorUpdateTemplate(MVKDevice* device, const VkDescriptorUpdateTemplateCreateInfoKHR* pCreateInfo);

	/** Destructor. */
	~MVKDescriptorUpdateTemplate() override = default;

protected:
	void propogateDebugName() override {}

	VkDescriptorUpdateTemplateTypeKHR _type;
	MVKVectorInline<VkDescriptorUpdateTemplateEntryKHR, 1> _entries;
};

#pragma mark -
#pragma mark Support functions

/** Updates the resource bindings in the descriptor sets inditified in the specified content. */
void mvkUpdateDescriptorSets(uint32_t writeCount,
							const VkWriteDescriptorSet* pDescriptorWrites,
							uint32_t copyCount,
							const VkCopyDescriptorSet* pDescriptorCopies);

/** Updates the resource bindings in the given descriptor set from the specified template. */
void mvkUpdateDescriptorSetWithTemplate(VkDescriptorSet descriptorSet,
										VkDescriptorUpdateTemplateKHR updateTemplate,
										const void* pData);

/**
 * If the shader stage binding has a binding defined for the specified stage, populates
 * the context at the descriptor set binding from the shader stage resource binding.
 */
void mvkPopulateShaderConverterContext(mvk::SPIRVToMSLConversionConfiguration& context,
									   MVKShaderStageResourceBinding& ssRB,
									   spv::ExecutionModel stage,
									   uint32_t descriptorSetIndex,
									   uint32_t bindingIndex,
									   MVKSampler* immutableSampler);
