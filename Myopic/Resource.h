#pragma once
#include <cstdint>
#include <type_traits>
#include <unordered_map>
#include <string>
#include <memory>
#include <iostream>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


typedef uint32_t RID; /* Resource ID*/

constexpr RID NULL_RID = 0;

class IResource {
public:
	IResource();

	~IResource() = default;

	inline bool operator==(IResource& r) {
		return _ID == r._ID;
	}
	inline bool operator!=(IResource& r) {
		return _ID != r._ID;
	}

	RID id() const {
		return _ID;
	}

private:
	RID _ID = 0;
};


class ref_count {
public:
	unsigned long AddRef() {
		return InterlockedIncrement(&count);
	}
	unsigned long RemoveRef() {
		return InterlockedDecrement(&count);
	}
	unsigned long Count() const {
		return count;
	}
private:
	volatile unsigned long count = 0;
};



template <typename T>
class resource_owner {
public:
	resource_owner(std::unique_ptr<T> ptr) : ptr(std::move(ptr)) {
	}

	T* get() {
		return ptr.get();
	}

	T* addref() {
		auto r = count.AddRef();
		std::cout << ptr << "++ : " << r << std::endl;
		return ptr.get();
	}

	void release() {
		auto r = count.RemoveRef();
		std::cout << ptr << "-- : " << r << std::endl;
	}

	unsigned long references() const {
		return count.Count();
	}
private:
	std::unique_ptr<T> ptr;
	ref_count count;
};


template <typename T>
class resource_ptr {
public:
	resource_ptr() : ptr(nullptr) {

	}
	resource_ptr(nullptr_t) : ptr(nullptr) {

	}
	resource_ptr(const resource_ptr& other) : ptr(other.ptr) {
		InternalAddRef();
	}
	resource_ptr& operator= (const resource_ptr&) = delete;

	resource_ptr(resource_ptr&& other) {
		swap(other);
	}

	resource_ptr& operator=(const resource_ptr&& p) {
		InternalRelease();
		this->ptr = p.ptr;
		if (this->ptr != nullptr) {
			this->ptr->addref();
		}

		return *this;
	}



	resource_ptr(resource_owner<T>& owner) : ptr(&owner) {
		InternalAddRef();
	}


	~resource_ptr() {
		InternalRelease();
	}

	void swap(resource_ptr& r)
	{
		resource_owner<T>* tmp = ptr;
		ptr = r.ptr;
		r.ptr = tmp;
	}


	T* operator->() {
		return ptr->get();
	}

	T* get() {
		return ptr->get();
	}

	void reset() {
		InternalRelease();
	}


	bool operator==(resource_ptr<T>& other) {
		return (ptr == other.ptr);
	}
	bool operator!=(resource_ptr<T>& other) {
		return (ptr != other.ptr);
	}

	bool operator==(nullptr_t) {
		return ptr == nullptr;
	}
	bool operator!=(nullptr_t) {
		return ptr != nullptr;
	}
private:
	resource_owner<T>* ptr;

	void InternalAddRef() {
		if (ptr != nullptr) {
			ptr->addref();
		}
	}

	void InternalRelease() {
		if (ptr != nullptr) {
			ptr->release();
			ptr = nullptr;
		}

	}
};

template <typename T>
class Registry {
	static_assert(std::is_base_of<IResource, T>::value);
public:

	void insert(const std::string& tag, std::unique_ptr<T> res) {
		RID rid = res->id();
		tag_map.insert_or_assign(tag, rid);
		auto v = map.insert_or_assign(rid, resource_owner<T>(std::move(res)));
		//map.insert({ res->id(), std::move(res)});
	}

	RID get_rid(const std::string& tag) {
		auto f = tag_map.find(tag);
		if (tag_map.contains(tag)) {
			return tag_map.at(tag);
		}
		return NULL_RID;
	}

	resource_ptr<T> get(const std::string& tag) {
		return get(get_rid(tag));
	}

	resource_ptr<T> get(RID id) {
		if (map.contains(id)) {
			return map.at(id);
		}
		return nullptr;
	}

private:
	std::unordered_map<std::string, RID> tag_map;
	std::unordered_map< RID, resource_owner<T>> map;
};

