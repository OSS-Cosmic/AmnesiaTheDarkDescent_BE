#include <cstddef>
#include <graphics/Image.h>
#include "bgfx/bgfx.h"
#include "bgfx/defines.h"
#include <cstdint>
#include <bx/debug.h>
#include <graphics/Bitmap.h>
#include <vector>
namespace hpl
{


    ImageDescriptor::ImageDescriptor(const ImageDescriptor& desc)
        : m_width(desc.m_width)
        , m_height(desc.m_height)
        , m_arraySize(desc.m_arraySize)
        , m_hasMipMaps(desc.m_hasMipMaps)
        , format(desc.format)
    {
    }
    ImageDescriptor::ImageDescriptor()
        : m_width(0)
        , m_height(0)
        , m_arraySize(1)
        , m_hasMipMaps(false)
        , format(bgfx::TextureFormat::Unknown)
    {
    }

    ImageDescriptor ImageDescriptor::CreateFromBitmap(const cBitmap& bitmap) {
        ImageDescriptor descriptor;
        descriptor.format = Image::FromHPLTextureFormat(bitmap.GetPixelFormat());
        descriptor.m_width = bitmap.GetWidth();
        descriptor.m_height = bitmap.GetHeight();
        descriptor.m_depth = bitmap.GetDepth();
        descriptor.m_hasMipMaps = bitmap.GetNumOfMipMaps() > 1;
        return descriptor;
    }

    ImageDescriptor ImageDescriptor::CreateTexture2D(uint16_t width, uint16_t height, bool hasMipMaps, bgfx::TextureFormat::Enum format) {
        ImageDescriptor desc;
        desc.m_width = width;
        desc.m_height = height;
        desc.m_hasMipMaps = hasMipMaps;
        desc.format = format;
        return desc;
    }

    ImageDescriptor ImageDescriptor::CreateTexture3D(uint16_t width, uint16_t height, uint16_t depth, bool hasMipMaps, bgfx::TextureFormat::Enum format) {
        ImageDescriptor desc;
        desc.m_width = width;
        desc.m_height = height;
        desc.m_depth = depth;
        desc.m_hasMipMaps = hasMipMaps;
        desc.format = format;
        return desc;
    }

    ImageDescriptor ImageDescriptor::CreateTexture2D(const char* name, uint16_t width, uint16_t height, bool hasMipMaps, bgfx::TextureFormat::Enum format) {
        ImageDescriptor desc = ImageDescriptor::CreateTexture2D(width, height, hasMipMaps, format);
        desc.m_name = name;
        return desc;
    }

    ImageDescriptor ImageDescriptor::CreateTexture3D(const char* name, uint16_t width, uint16_t height, uint16_t depth, bool hasMipMaps, bgfx::TextureFormat::Enum format) {
        ImageDescriptor desc = ImageDescriptor::CreateTexture3D(width, height, depth, hasMipMaps, format);
        desc.m_name = name;
        return desc;
    }

    Image::Image() :
        iResourceBase("", _W(""), 0)
    {
    }

    Image::Image(const tString& asName, const tWString& asFullPath):
        iResourceBase(asName, asFullPath, 0)
    {
    }

    bool Image::Reload() {
        return false;
    }
    
    void Image::Unload() {
    }

    void Image::Destroy() {
    }

    Image::Image(Image&& other) :
        iResourceBase(other.GetName(), other.GetFullPath(), 0) {
        m_handle = other.m_handle;
        other.m_handle = BGFX_INVALID_HANDLE;
    }

    Image::~Image()
    {
        if (bgfx::isValid(m_handle))
        {
            bgfx::destroy(m_handle);
        }
    }

    void Image::operator=(Image&& other) {
        m_handle = other.m_handle;
        other.m_handle = BGFX_INVALID_HANDLE;
    }

    void Image::Initialize(const ImageDescriptor& descriptor, const bgfx::Memory* mem) {
        BX_ASSERT(!bgfx::isValid(m_handle), "RenderTarget already initialized");
        m_width = descriptor.m_width;
        m_height = descriptor.m_height;

        uint64_t flags = (descriptor.m_configuration.m_uClamp ? BGFX_SAMPLER_U_CLAMP : 0) |
            (descriptor.m_configuration.m_vClamp ? BGFX_SAMPLER_V_CLAMP : 0) |
            (descriptor.m_configuration.m_wClamp ? BGFX_SAMPLER_W_CLAMP : 0) |
            [&] () -> uint64_t {
                switch(descriptor.m_configuration.m_rt) {
                    case RTType::RT_Write:
                        return BGFX_TEXTURE_RT;
                    case RTType::RT_WriteOnly:
                        return BGFX_TEXTURE_RT_WRITE_ONLY;
                    case RTType::RT_MSAA_X2:
                        return BGFX_TEXTURE_RT_MSAA_X2;
                    case RTType::RT_MSAA_X4:
                        return BGFX_TEXTURE_RT_MSAA_X4;
                    case RTType::RT_MSAA_X8:
                        return BGFX_TEXTURE_RT_MSAA_X8;
                    case RTType::RT_MSAA_X16:
                        return BGFX_TEXTURE_RT_MSAA_X16;
                    default:
                        break;
                }
                return 0;
            }();
        if(descriptor.m_isCubeMap) {
            BX_ASSERT(descriptor.m_width == descriptor.m_height, "Cube map must be square");
            m_handle = bgfx::createTextureCube(descriptor.m_width, 
                descriptor.m_hasMipMaps,
                 descriptor.m_arraySize,
                 descriptor.format, 
                    flags, 
                    mem);

        }
        else if (descriptor.m_depth > 1)
        {
            m_handle = bgfx::createTexture3D(
                descriptor.m_width,
                descriptor.m_height,
                descriptor.m_depth,
                descriptor.m_hasMipMaps,
                descriptor.format,
                flags,
                mem);
        }
        else
        {
            m_handle = bgfx::createTexture2D(
                descriptor.m_width, 
                descriptor.m_height, 
                descriptor.m_hasMipMaps, 
                descriptor.m_arraySize, 
                descriptor.format, 
                flags, 
                mem);
        }
        if(descriptor.m_name) {
            bgfx::setName(m_handle, descriptor.m_name);
        }
    }
    
    void Image::Invalidate() {
        if (bgfx::isValid(m_handle))
        {
            bgfx::destroy(m_handle);
        }
        m_handle = BGFX_INVALID_HANDLE;
    }

    // const ImageDescriptor& Image::GetDescriptor() const
    // {
    //     return m_descriptor;
    // }

    bgfx::TextureFormat::Enum Image::FromHPLTextureFormat(ePixelFormat format)
    {
        switch (format)
        {
        case ePixelFormat_Alpha:
            return bgfx::TextureFormat::A8;
        case ePixelFormat_Luminance:
            return bgfx::TextureFormat::R8;
        case ePixelFormat_LuminanceAlpha:
            return bgfx::TextureFormat::RG8;
        case ePixelFormat_RGB:
            return bgfx::TextureFormat::RGB8;
        case ePixelFormat_RGBA:
            return bgfx::TextureFormat::RGBA8;
        case ePixelFormat_BGRA:
            return bgfx::TextureFormat::BGRA8;
        case ePixelFormat_DXT1:
            return bgfx::TextureFormat::BC1;
        case ePixelFormat_DXT2:
        case ePixelFormat_DXT3:
            return bgfx::TextureFormat::BC2;
        case ePixelFormat_DXT4:
        case ePixelFormat_DXT5: // DXT4 and DXT5 are the same
            return bgfx::TextureFormat::BC3;
        case ePixelFormat_Depth16:
            return bgfx::TextureFormat::D16;
        case ePixelFormat_Depth24:
            return bgfx::TextureFormat::D24;
        case ePixelFormat_Depth32:
            return bgfx::TextureFormat::D32;
        case ePixelFormat_Alpha16:
            return bgfx::TextureFormat::R16F;
        case ePixelFormat_Luminance16:
            return bgfx::TextureFormat::R16F;
        case ePixelFormat_LuminanceAlpha16:
            return bgfx::TextureFormat::RG16F;
        case ePixelFormat_RGBA16:
            return bgfx::TextureFormat::RGBA16F;
        case ePixelFormat_Alpha32:
            return bgfx::TextureFormat::R32F;
        case ePixelFormat_Luminance32:
            return bgfx::TextureFormat::R32F;
        case ePixelFormat_LuminanceAlpha32:
            return bgfx::TextureFormat::RG32F;
        case ePixelFormat_RGB32:
            break;
        case ePixelFormat_RGBA32:
            return bgfx::TextureFormat::RGBA32F;
        case ePixelFormat_RGB16:
            return bgfx::TextureFormat::BC6H;
        case ePixelFormat_BGR:
            return bgfx::TextureFormat::RGB8;
        default:
            BX_ASSERT(false, "Unsupported texture format: %d", format)
            break;
        }
        return bgfx::TextureFormat::Unknown;
    }


    void Image::InitializeCubemapFromBitmaps(Image& image, const absl::Span<cBitmap*> bitmaps, const ImageDescriptor& desc) {
        BX_ASSERT(bitmaps.size() == 6, "Cubemap must have 6 bitmaps");
        size_t size = 0;
        for(auto& bitmap : bitmaps) {
            BX_ASSERT(bitmap->GetNumOfImages() == 1, "Only single image bitmaps are supported");
            if(desc.m_hasMipMaps) {
                for(auto mipIndex = 0; mipIndex < bitmap->GetNumOfMipMaps(); ++mipIndex) {
                    size += bitmap->GetData(0, mipIndex)->mlSize;
                }
            } else {
                size += bitmap->GetData(0, 0)->mlSize;
            }
        }
        auto* memory = bgfx::alloc(size);
        size_t offset = 0;
        if(desc.m_hasMipMaps) {
            for(auto& bitmap : bitmaps) {
                for(auto mipIndex = 0; mipIndex < bitmap->GetNumOfMipMaps(); ++mipIndex) {
                    auto data = bitmap->GetData(0, mipIndex);
                    std::copy(data->mpData, data->mpData + data->mlSize, memory->data + offset);
                    offset += data->mlSize;
                }
            }
        } else {
            for(auto& bitmap : bitmaps) {
                auto data = bitmap->GetData(0, 0);
                std::copy(data->mpData, data->mpData + data->mlSize, memory->data + offset);
                offset += data->mlSize;
            }
        }

        image.Initialize(desc, memory);
    }

    void Image::InitializeFromBitmap(Image& image, cBitmap& bitmap, const ImageDescriptor& desc) {
        if(desc.m_isCubeMap) {
            BX_ASSERT(bitmap.GetNumOfImages() == 6, "Cube map must have 6 images");
            
            auto* memory = bgfx::alloc([&]() {
                if(desc.m_hasMipMaps) {
                    size_t size = 0;
                    for(size_t i = 0; i < bitmap.GetNumOfMipMaps(); ++i) {
                        size += bitmap.GetData(0, i)->mlSize;
                    }
                    return size * static_cast<size_t>(bitmap.GetNumOfImages());
                }
                return static_cast<size_t>(bitmap.GetNumOfImages()) * static_cast<size_t>(bitmap.GetData(0, 0)->mlSize);
            }());


            size_t offset = 0;
            if(desc.m_hasMipMaps) {
                for(auto imageIdx = 0; imageIdx < bitmap.GetNumOfImages(); ++imageIdx) {
                    for(auto mipIndex = 0; mipIndex < bitmap.GetNumOfMipMaps(); ++mipIndex) {
                        auto data = bitmap.GetData(imageIdx, mipIndex);
                        std::copy(data->mpData, data->mpData + data->mlSize, memory->data + offset);
                        offset += data->mlSize;
                    }
                }
            } else {
                for(auto i = 0; i < bitmap.GetNumOfImages(); ++i) {
                    auto data = bitmap.GetData(i, 0);
                    std::copy(data->mpData, data->mpData + data->mlSize, memory->data + offset);
                    offset += data->mlSize;
                }
            }
            image.Initialize(desc, memory);
            return;
        }
        
        if(desc.m_hasMipMaps) {
            auto* memory = bgfx::alloc([&]() {
                size_t size = 0;
                for(size_t i = 0; i < bitmap.GetNumOfMipMaps(); ++i) {
                    size += bitmap.GetData(0, i)->mlSize;
                }
                return size;
            }());

            size_t offset = 0;
            for(auto i = 0; i < bitmap.GetNumOfMipMaps(); ++i) {
                auto data = bitmap.GetData(0, i);
                std::copy(data->mpData, data->mpData + data->mlSize, memory->data + offset);
                offset += data->mlSize;
            }
            image.Initialize(desc, memory);
            return;
        }

        auto data = bitmap.GetData(0, 0);
        image.Initialize(desc, bgfx::copy(data->mpData, data->mlSize));
    }

    bgfx::TextureHandle Image::GetHandle() const
    {
        return m_handle;
    }
} // namespace hpl