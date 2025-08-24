namespace Validity {
    template <typename T>
    bool IsBadPoint(T* ptr)
    {
        std::uintptr_t pointer = reinterpret_cast<std::uintptr_t>(ptr);
        return (pointer < 0xFFFFFFFFFFULL) || (pointer > 0x2FFFFFFFFFFULL);
    }
}