#ifndef META_HPP
#define META_HPP

#include <tuple>
#include <variant>
#include "Common.hpp"

namespace mp
{
namespace detail
{
struct IsValid
{
    template <typename F, typename... Args,
              typename = decltype(std::declval<F>()(std::declval<Args>()...))>
    static constexpr auto impl(int) { return std::true_type{}; }
    template <typename F, typename... Args>
    static constexpr auto impl(float) { return std::false_type{}; }
    template <typename F>
    struct checker_type
    {
        template <typename... Args>
        constexpr auto operator()(Args &&... args) const { return impl<F, Args...>(0); }
    };
    template <typename F>
    constexpr auto operator()(F &&) const { return checker_type<F>{}; }
};
} // namespace detail
constexpr inline detail::IsValid is_valid;

template <typename... Ts>
struct Tuple : std::tuple<Ts...>
{
    using base_type = std::tuple<Ts...>;
    using base_type::base_type;

    constexpr Tuple(const base_type &tp) : base_type(tp) {}
    constexpr Tuple(base_type &&tp) : base_type(std::move(tp)) {}

    template <typename Pred, size_t... Index>
    constexpr void _for_each_impl(Pred pred, std::index_sequence<Index...>)
    {
        using dummy = int[];
        void(dummy{(pred(std::get<Index>(*this)), 0)..., 0});
    }
    template <typename Pred>
    constexpr void for_each(Pred pred)
    {
        _for_each_impl(pred, std::make_index_sequence<sizeof...(Ts)>());
    }
    // copy and paste...
    template <typename Pred, size_t... Index>
    constexpr void _for_each_impl(Pred pred, std::index_sequence<Index...>) const
    {
        using dummy = int[];
        void(dummy{(pred(std::get<Index>(*this)), 0)..., 0});
    }
    template <typename Pred>
    constexpr void for_each(Pred pred) const
    {
        _for_each_impl(pred, std::make_index_sequence<sizeof...(Ts)>());
    }
};

template <int First, int Second>
struct Pair
{
    const static int first = First;
    const static int second = Second;
};

template <typename... Ts>
struct Variant : std::variant<Ts...>
{
    using base_type = std::variant<Ts...>;

    template <typename T>
    Variant(T &&val) : base_type(std::forward<T>(val)) {}
    Variant(const base_type &var) : base_type(var) {}
    Variant(base_type &&var) : base_type(std::move(var)) {}

    template <size_t Index, typename Pred>
    void _apply_impl(Pred pred)
    {
        // inorder to avoid CE, if pred doesn't return anything
        // the user need to add required operations to pred's return type
        // e.g., pred = [](auto x)->std::void_t<decltype(opt(x))>{opt(x);};
        if constexpr (Index == sizeof...(Ts))
            return;
        else
        {
            auto can_apply = is_valid([](auto &&x) -> decltype(pred(x)) {});
            if (base_type::index() == Index)
            {
                if constexpr (decltype(can_apply(std::get<Index>(*this)))::value)
                    pred(std::get<Index>(*this));
            }
            _apply_impl<Index + 1>(pred);
        }
    }
    template <typename Pred>
    void apply(Pred pred)
    {
        _apply_impl<0>(pred);
    }
    // copy and paste...
    template <size_t Index, typename Pred>
    void _apply_impl(Pred pred) const
    {
        // inorder to avoid CE, if pred doesn't return anything
        // the user need to add required operations to pred's return type
        // e.g., pred = [](auto x)->std::void_t<decltype(opt(x))>{opt(x);};
        if constexpr (Index == sizeof...(Ts))
            return;
        else
        {
            auto can_apply = is_valid([](auto &&x) -> decltype(pred(x)) {});
            if (base_type::index() == Index)
            {
                if constexpr (decltype(can_apply(std::get<Index>(*this)))::value)
                    pred(std::get<Index>(*this));
            }
            _apply_impl<Index + 1>(pred);
        }
    }
    template <typename Pred>
    void apply(Pred pred) const
    {
        _apply_impl<0>(pred);
    }
    template <typename OS>
    friend decltype(auto) operator<<(std::ostream &os, const Variant &var)
    {
        var.apply([&](auto &&x) -> std::void_t<decltype(os << x)> { os << x; });
        return std::forward<OS>(os);
    }
};
} // namespace mp

#endif // META_HPP