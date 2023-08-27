#ifndef AOP_HPP
#define AOP_HPP

//from cosmos

#include<type_traits>
#include<functional>
#include<string>

namespace coral {

#define HAS_MEMBER(member)                                                                                    \
template <typename T, typename... Args>                                                                       \
struct has_member_##member                                                                                    \
{                                                                                                             \
private:                                                                                                      \
    template <typename U>                                                                                     \
    static auto Check(int) -> decltype(std::declval<U>().member(std::declval<Args>()...), std::true_type());  \
    template <typename U>                                                                                     \
    static std::false_type Check(...);                                                                        \
public:                                                                                                       \
    enum {value = std::is_same<decltype(Check<T>(0)), std::true_type>::value};                                \
};                                                                                                            \


HAS_MEMBER(Before)
HAS_MEMBER(After)

template <typename Func, typename... Args>
struct Aspect
{
	Aspect(Func&& f) : m_func(std::forward<Func>(f))
	{

	}

	template <typename T>
	typename std::enable_if<has_member_Before<T, Args...>::value&& has_member_After<T, Args...>::value>::type
		Invoke(Args&&... args, T&& aspect)
	{
		aspect.Before(std::forward<Args>(args)...);
		m_func(std::forward<Args>(args)...);
		aspect.After(std::forward<Args>(args)...);
	}

	template <typename T>
	typename std::enable_if<has_member_Before<T, Args...>::value && !has_member_After<T, Args...>::value>::type
		Invoke(Args&&... args, T&& aspect)
	{
		aspect.Before(std::forward<Args>(args)...);
		m_func(std::forward<Args>(args)...);
	}

	template <typename T>
	typename std::enable_if<!has_member_Before<T, Args...>::value&& has_member_After<T, Args...>::value>::type
		Invoke(Args&&... args, T&& aspect)
	{
		m_func(std::forward<Args>(args)...);
		aspect.After(std::forward<Args>(args)...);
	}

	template <typename Head, typename... Tail>
	typename std::enable_if<has_member_Before<Head, Args...>::value&& has_member_After<Head, Args...>::value>::type
		Invoke(Args&&... args, Head&& headAspect, Tail&&... tailAspect)
	{
		headAspect.Before(std::forward<Args>(args)...);
		Invoke(std::forward<Args>(args)..., std::forward<Tail>(tailAspect)...);
		headAspect.After(std::forward<Args>(args)...);
	}

	template <typename Head, typename... Tail>
	typename std::enable_if<has_member_Before<Head, Args...>::value && !has_member_After<Head, Args...>::value>::type
		Invoke(Args&&... args, Head&& headAspect, Tail&&... tailAspect)
	{
		headAspect.Before(std::forward<Args>(args)...);
		Invoke(std::forward<Args>(args)..., std::forward<Tail>(tailAspect)...);
	}

	template <typename Head, typename... Tail>
	typename std::enable_if<!has_member_Before<Head, Args...>::value&& has_member_After<Head, Args...>::value>::type
		Invoke(Args&&... args, Head&& headAspect, Tail&&... tailAspect)
	{
		Invoke(std::forward<Args>(args)..., std::forward<Tail>(tailAspect)...);
		headAspect.After(std::forward<Args>(args)...);
	}

private:
	Func m_func;
};

//外部接口
template<typename... AP, typename... Args, typename Func>
void Invoke(Func&& f, Args&&... args)
{
	Aspect<Func, Args...> asp(std::forward<Func>(f));
	asp.Invoke(std::forward<Args>(args)..., AP()...);
}

} //namespace coral

#endif //AOP_HPP