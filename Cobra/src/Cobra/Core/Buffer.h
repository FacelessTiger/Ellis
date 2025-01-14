#pragma once

#include <stdint.h>
#include <cstring>

namespace Cobra {

	// Non-owning raw buffer
	struct Buffer
	{
		uint8_t* Data = nullptr;
		uint64_t Size = 0;

		Buffer() = default;
		Buffer(const Buffer&) = default;

		Buffer(uint64_t size)
		{
			Allocate(size);
		}

		Buffer(const void* data, uint64_t size)
			: Data((uint8_t*)data), Size(size)
		{ }

		static Buffer Copy(Buffer other)
		{
			Buffer result(other.Size);

			memcpy(result.Data, other.Data, other.Size);
			return result;
		}

		void Allocate(uint64_t size)
		{
			Release();

			Data = (uint8_t*)malloc(size);
			Size = size;
		}

		void Release()
		{
			free(Data);
			Data = nullptr;
			Size = 0;
		}

		template<typename T>
		T* As()
		{
			return (T*)Data;
		}

		operator bool() const
		{
			return (bool)Data;
		}
	};

	class ScopedBuffer
	{
	private:
		Buffer m_Buffer;
	public:
		ScopedBuffer(Buffer buffer)
			: m_Buffer(buffer)
		{ }

		ScopedBuffer(uint64_t size)
			: m_Buffer(size)
		{ }

		~ScopedBuffer()
		{
			m_Buffer.Release();
		}

		uint8_t* Data() { return m_Buffer.Data; }
		uint64_t Size() { return m_Buffer.Size; }

		template<typename T>
		T* As()
		{
			return m_Buffer.As<T>();
		}

		operator bool() const { return m_Buffer.Data; }
	};

}