﻿#ifndef HGL_GRAPH_VULKAN_MATERIAL_INCLUDE
#define HGL_GRAPH_VULKAN_MATERIAL_INCLUDE

#include"VK.h"
#include<hgl/type/Map.h>
#include<hgl/type/BaseString.h>
VK_NAMESPACE_BEGIN
class Device;
class ShaderModule;
class VertexShaderModule;
class DescriptorSetLayoutCreater;
class DescriptorSetLayout;
class MaterialInstance;
class VertexAttributeBinding;
class VertexBuffer;
class Renderable;

using ShaderModuleMap=hgl::Map<VkShaderStageFlagBits,const ShaderModule *>;

/**
 * 材质类<br>
 * 用于管理shader，提供DescriptorSetLayoutCreater
 */
class Material
{
    VkDevice device;
    ShaderModuleMap *shader_maps;
    VertexShaderModule *vertex_sm;
    List<VkPipelineShaderStageCreateInfo> shader_stage_list;

    DescriptorSetLayoutCreater *dsl_creater;

public:

    Material(Device *dev,ShaderModuleMap *smm);
    ~Material();

    const int GetUBOBinding(const UTF8String &)const;
    const int GetVBOBinding(const UTF8String &)const;

    MaterialInstance *CreateInstance()const;

    const uint32_t                          GetStageCount           ()const{return shader_stage_list.GetCount();}
    const VkPipelineShaderStageCreateInfo * GetStages               ()const{return shader_stage_list.GetData();}
};//class Material

/**
 * 材质实例<br>
 * 用于在指定Material的情况下，具体绑定UBO/TEXTURE等，提供pipeline
 */
class MaterialInstance
{
    const Material *mat;                                                                            ///<这里的是对material的完全引用，不做任何修改
    const VertexShaderModule *vertex_sm;
    VertexAttributeBinding *vab;
    DescriptorSetLayout *desc_set_layout;

public:

    MaterialInstance(const Material *m,const VertexShaderModule *,VertexAttributeBinding *v,DescriptorSetLayout *d);
    ~MaterialInstance();

    bool UpdateUBO(const uint32_t binding,const VkDescriptorBufferInfo *buf_info);
    bool UpdateUBO(const UTF8String &name,const VkDescriptorBufferInfo *buf_info)
    {
        if(name.IsEmpty()||!buf_info)
            return(false);

        return UpdateUBO(mat->GetUBOBinding(name),buf_info);
    }

    const VertexShaderModule *              GetVertexShader ()const{return vertex_sm;}

    const uint32_t                          GetStageCount   ()const{return mat->GetStageCount();}
    const VkPipelineShaderStageCreateInfo * GetStages       ()const{return mat->GetStages();}

    void Write(VkPipelineVertexInputStateCreateInfo &vis)const;

    DescriptorSetLayout *   GetDSL(){return desc_set_layout;}

    Renderable *CreateRenderable();
};//class MaterialInstance
VK_NAMESPACE_END
#endif//HGL_GRAPH_VULKAN_MATERIAL_INCLUDE
