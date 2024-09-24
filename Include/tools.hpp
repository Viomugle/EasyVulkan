#ifndef _TOOLS_HPP_
#define _TOOLS_HPP_

#include <vulkan/vulkan.h>
#include <type_traits>

namespace vulkan {
#define DestroyHandleBy(Func) if (handle) { Func(graphicsBase::Base().Device(), handle, nullptr); handle = VK_NULL_HANDLE; }
#define MoveHandle handle = other.handle; other.handle = VK_NULL_HANDLE;
#define DefineHandleTypeOperator operator decltype(handle)() const { return handle; }
#define DefineAddressFunction const decltype(handle)* Address() const { return &handle; }
#define ExecuteOnce(...) { static bool executed = false; if (executed) return __VA_ARGS__; executed = true; }
    inline auto &outStream = std::cout;


    using commandBuffer=VkCommandBuffer;
    using commandPool=VkCommandPool;
    using renderPass=VkRenderPass;
    using pipelineLayout=VkPipelineLayout;
    using pipeline=VkPipeline;
    using framebuffer=VkFramebuffer;
    using shaderModule=VkShaderModule;

    constexpr struct outStream_t {
        static std::stringstream ss;

        struct returnedStream_t {
            returnedStream_t operator<<(const std::string &string) const {
                ss << string;
                return {};
            }
        };

        returnedStream_t operator<<(const std::string &string) const {
            ss.clear();
            ss << string;
            return {};
        }
    }outStream;
    inline std::stringstream outStream_t::s;



#ifndef VK_RESULT_THROW

    class result_t {
        VkResult result;
    public :
        static void (*callback_throw)(VkResult);

        result_t(VkResult result) : result(result) {};

        result_t(result_t &&other) noexcept: result(other.result) { other.result = VK_SUCCESS; };

        ~result_t() noexcept(false) {
            if (uint32_t(result) < VK_RESULT_MAX_ENUM)
                return;
            if (callback_throw)
                callback_throw(result);
            throw result;
        }

        operator VkResult() {
            VkResult result = this->result;
            this->result = VK_SUCCESS;
            return result;
        }
    };

    inline void (*result_t::callback_throw)(VkResult);

#elif defined(VK_RESULT_NODISCARD)
    struct [[nodiscard]] result_t{
        VkResult result;
        reesult_t (VkResult result):result(result){}
        operator  VkResult ()const {return result;}
    };

#pragma warning (disable:4834)
#pragma warning (disable:6031)
#else
    using result_t =VkResult;
#endif

    template<typename T>
    class arrayRef {
        T *const pArray = nullptr;
        size_t count = 0;
    public :
        arrayRef() = default;

        arrayRef(T &data) : pArray(&data), count(1) {}

        template<size_t elementCount>
        arrayRef(T(&data)[elementCount]):pArray(data), count(elementCount) {}

        arrayRef(T *pData, size_t elementCount) : pArray(pData), count(elementCount) {}

        arrayRef(const arrayRef<std::remove_const_t<T>> &other) : pArray(other.Pointer()), count(other.Count()) {}

        T *Pointer() const { return pArray; }

        size_t Count() const { return count; }

        T &operator[](size_t index) const { return pArray[index]; }

        T *begin() const { return pArray; }

        T *end() const { return pArray + count; }

        arrayRef &operator=(const arrayRef) = delete;

    };

    result_t::callback_throw=[](VkResult result){
        MessageBoxA(hWindow, outStream.ss.str().c_str(), nullptr, MB_OK);
    };


};


#endif