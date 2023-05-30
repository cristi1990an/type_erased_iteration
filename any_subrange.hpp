#include <type_traits>
#include <concepts>

template<typename ReferenceType>
struct dynamic_iterator_interface
{
private:
	static constexpr bool can_be_written_to =
		!std::is_rvalue_reference_v<ReferenceType> &&
		!std::is_const_v<ReferenceType>;

public:
	using iterator_category = std::conditional_t<can_be_written_to,
		std::forward_iterator_tag, std::input_iterator_tag>;
	using value_type = std::remove_cvref_t<ReferenceType>;
	using difference_type = std::ptrdiff_t;
	using pointer = std::remove_reference_t<ReferenceType>*;
	using reference = ReferenceType;

	constexpr virtual dynamic_iterator_interface* get_copy() const = 0;

	constexpr virtual reference operator*() const = 0;
	constexpr virtual dynamic_iterator_interface& operator++() = 0;

	constexpr virtual bool operator==(const dynamic_iterator_interface&) const = 0;

	constexpr virtual ~dynamic_iterator_interface() = default;
};

template <typename Iterator>
concept non_output_iterator = true; // TODO


template <non_output_iterator Iterator, typename ReferenceType = std::iter_reference_t<Iterator>>
class underlying_iterator final : public dynamic_iterator_interface<ReferenceType>
{
private:
	using base_t = dynamic_iterator_interface<ReferenceType>;
public:
	constexpr underlying_iterator() = default;
	explicit constexpr underlying_iterator(Iterator it)
		: iter{ it }
	{

	}

	constexpr base_t* get_copy() const override
	{
		return new underlying_iterator{ iter };
	}

	constexpr base_t::reference operator*() const override
	{
		return *iter;
	}

	constexpr virtual underlying_iterator& operator++() override
	{
		++iter;
		return *this;
	}

	constexpr virtual bool operator==(const base_t& other) const override
	{
		const underlying_iterator* otherUnderlying = dynamic_cast<const underlying_iterator*>(std::addressof(other));

		if (!otherUnderlying)
		{
			return false;
		}

		return iter == otherUnderlying->iter;
	}

private:
	Iterator iter;
};

template<typename ReferenceType>
class dynamic_iterator
{
private:
	static constexpr bool can_be_written_to =
		!std::is_rvalue_reference_v<ReferenceType> &&
		!std::is_const_v<ReferenceType>;

public:
	using iterator_category = std::conditional_t<can_be_written_to,
		std::forward_iterator_tag, std::input_iterator_tag>;
	using value_type = std::remove_cvref_t<ReferenceType>;
	using difference_type = std::ptrdiff_t;
	using pointer = std::remove_reference_t<ReferenceType>*;
	using reference = ReferenceType;

	constexpr dynamic_iterator() noexcept = default;
	constexpr dynamic_iterator(const dynamic_iterator& other)
	{
		if (other.iter)
		{
			iter = other.iter->get_copy();
		}
	}
	constexpr dynamic_iterator(dynamic_iterator&& other) noexcept
		: iter{ std::exchange(other.iter, nullptr) }
	{

	}
	constexpr dynamic_iterator& operator=(const dynamic_iterator& other)
	{
		if (this == std::addressof(other))
		{
			return *this;
		}

		delete iter;

		if (other.iter)
		{
			iter = other.iter->get_copy();
		}
		else
		{
			iter = nullptr;
		}
		return *this;
	}
	constexpr dynamic_iterator& operator=(dynamic_iterator&& other)
	{
		if (this == std::addressof(other))
		{
			return *this;
		}

		delete iter;

		iter = std::exchange(other.iter, nullptr);
		return *this;
	}

	template <non_output_iterator Iterator>
		requires (!std::same_as<dynamic_iterator, Iterator>)
	constexpr dynamic_iterator(Iterator it)
		: iter{ new underlying_iterator<Iterator, ReferenceType>{it} }
	{

	}

	constexpr ~dynamic_iterator()
	{
		delete iter;
	}

	constexpr reference operator*() const
	{
		if (!iter)
		{
			throw std::runtime_error("Something");
		}

		return iter->operator*();
	}

	constexpr dynamic_iterator& operator++()
	{
		if (iter)
		{
			++(*iter);
		}
		return *this;
	}
	constexpr dynamic_iterator operator++(int)
	{
		if (!iter)
		{
			return (*this);
		}

		auto tmp = *this;
		++(*iter);
		return tmp;
	}

	constexpr bool operator==(const dynamic_iterator& other) const
	{
		if (!iter && !other.iter)
		{
			return true;
		}
		if (!iter)
		{
			return false;
		}
		if (!other.iter)
		{
			return false;
		}

		return *iter == *other.iter;
	}

private:
	dynamic_iterator_interface<ReferenceType>* iter = nullptr;
};

template <typename Iterator>
dynamic_iterator(Iterator) -> dynamic_iterator<std::iter_reference_t<Iterator>>;


template <typename ReferenceType>
struct any_subrange
{
public:
	any_subrange() = default;

	template <std::ranges::range Range>
		requires (!std::same_as< any_subrange, std::remove_cvref_t<Range>>)
	explicit constexpr any_subrange(Range&& range)
		: begin_{ std::ranges::begin(range) }
		, end_{ std::ranges::end(range) }
	{

	}

	template <std::ranges::range Range>
		requires (!std::same_as<any_subrange, std::remove_cvref_t<Range>>)
	constexpr any_subrange& operator=(Range&& range)
	{
		begin_ = std::ranges::begin(range);
		end_ = std::ranges::end(range);

		return *this;
	}

	constexpr auto begin() const
	{
		return begin_;
	}
	constexpr auto end() const
	{
		return end_;
	}

private:
	dynamic_iterator<ReferenceType> begin_;
	dynamic_iterator<ReferenceType> end_;
};

