﻿#include<hgl/graph/vulkan/VKMaterial.h>
#include<hgl/graph/vulkan/VKDevice.h>
#include<hgl/graph/vulkan/VKDescriptorSets.h>
#include<hgl/graph/vulkan/VKShaderModule.h>
#include<hgl/graph/vulkan/VKVertexAttributeBinding.h>
#include<hgl/graph/vulkan/VKRenderable.h>
#include<hgl/graph/vulkan/VKBuffer.h>
VK_NAMESPACE_BEGIN
Material *CreateMaterial(Device *dev,ShaderModuleMap *shader_maps)
{
    DescriptorSetLayoutCreater *dsl_creater=new DescriptorSetLayoutCreater(dev);

    const int shader_count=shader_maps->GetCount();

    List<VkPipelineShaderStageCreateInfo> *shader_stage_list=new List<VkPipelineShaderStageCreateInfo>;

    shader_stage_list->SetCount(shader_count);

    VkPipelineShaderStageCreateInfo *p=shader_stage_list->GetData();

    VertexShaderModule *vertex_sm;
    const ShaderModule *sm;
    auto **itp=shader_maps->GetDataList();
    for(int i=0;i<shader_count;i++)
    {
        sm=(*itp)->right;

        if(sm->GetStage()==VK_SHADER_STAGE_VERTEX_BIT)
            vertex_sm=(VertexShaderModule *)sm;

        memcpy(p,sm->GetCreateInfo(),sizeof(VkPipelineShaderStageCreateInfo));
        ++p;
        ++itp;

        {
            const ShaderBindingList &ubo_list=sm->GetUBOBindingList();

            dsl_creater->BindUBO(ubo_list.GetData(),ubo_list.GetCount(),sm->GetStage());
        }
    }

    VertexAttributeBinding *vab=vertex_sm->CreateVertexAttributeBinding();

    DescriptorSetLayout *dsl=dsl_creater->Create();

    if(dsl)
    {
        return(new Material(dev,shader_maps,vertex_sm,shader_stage_list,dsl_creater,dsl,vab));
    }

    return(nullptr);
}

Material::Material(Device *dev,ShaderModuleMap *smm,VertexShaderModule *vsm,List<VkPipelineShaderStageCreateInfo> *psci_list,DescriptorSetLayoutCreater *dslc,DescriptorSetLayout *dsl,VertexAttributeBinding *v)
{
    device=*dev;
    shader_maps=smm;
    vertex_sm=vsm;
    shader_stage_list=psci_list;
    dsl_creater=dslc;
    desc_set_layout=dsl;
    vab=v;
}

Material::~Material()
{
    delete vab;
    
    delete desc_set_layout;
    delete dsl_creater;

    const int count=shader_stage_list->GetCount();

    if(count>0)
    {
        VkPipelineShaderStageCreateInfo *ss=shader_stage_list->GetData();
        for(int i=0;i<count;i++)
        {
            vkDestroyShaderModule(device,ss->module,nullptr);
            ++ss;
        }
    }

    delete shader_stage_list;
    delete shader_maps;
}

const int Material::GetUBOBinding(const UTF8String &name)const
{
    int result;
    const int shader_count=shader_maps->GetCount();

    const ShaderModule *sm;
    auto **itp=shader_maps->GetDataList();
    for(int i=0;i<shader_count;i++)
    {
        sm=(*itp)->right;
        result=sm->GetUBO(name);
        if(result!=-1)
            return result;

        ++itp;
    }

    return(-1);
}

const int Material::GetVBOBinding(const UTF8String &name)const
{
    if(!vertex_sm)return(-1);

    return vertex_sm->GetBinding(name);
}

const VkPipelineLayout Material::GetPipelineLayout()const
{
    return desc_set_layout->GetPipelineLayout();
}

const uint32_t Material::GetDescriptorSetCount()const
{
    return desc_set_layout->GetCount();
}

const VkDescriptorSet *Material::GetDescriptorSets()const
{
    return desc_set_layout->GetDescriptorSets();
}

bool Material::UpdateUBO(const uint32_t binding,const VkDescriptorBufferInfo *buf_info)
{
    VkDescriptorSet set=desc_set_layout->GetDescriptorSet(binding);

    VkWriteDescriptorSet writeDescriptorSet = {};

    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet = set;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeDescriptorSet.pBufferInfo = buf_info;
    writeDescriptorSet.dstBinding = binding;

    vkUpdateDescriptorSets(device,1,&writeDescriptorSet,0,nullptr);
    return(true);
}

void Material::Write(VkPipelineVertexInputStateCreateInfo &vis)const
{
    return vab->Write(vis);
}

Renderable *Material::CreateRenderable()
{
    return(new Renderable(vertex_sm));
}
VK_NAMESPACE_END
