#pragma once
#include <cstdint>
#include <type_traits>
#include <unordered_map>
#include <string>
#include <memory>
#include <iostream>

#include <atomic>


typedef uint32_t RID; /* Resource ID*/

constexpr RID NULL_RID = 0;

class ref_count {
public:

	ref_count& operator=(const ref_count& other) {
		this->count.exchange(other.count);
		return *this;
	}

	unsigned long AddRef() {
		return ++count;
	}
	unsigned long RemoveRef() {
		return --count;
	}
	unsigned long Count() const {
		return count;
	}
private:
	std::atomic<unsigned long> count = 0;
};


class IResource {
public:
	IResource();

	~IResource() {
		std::cout << "delete " << _id << std::endl;
	}

	inline bool operator==(IResource& r) {
		return _id == r._id;
	}
	inline bool operator!=(IResource& r) {
		return _id != r._id;
	}

	inline RID id() const {
		return _id;
	}

	inline unsigned long addref() {
		auto r = _count.AddRef();
		std::cout << _id << "++ : " << r << std::endl;
		return r;
	}

	inline unsigned long release() {
		auto r = _count.RemoveRef();
		std::cout << _id << "-- : " << r << std::endl;
		return r;
	}


private:
	RID _id = 0;
	ref_count _count;
};





template <typename T>
class resource_ptr {
public:

	resource_ptr(resource_ptr&& other) noexcept : ptr(other.ptr) {
		InternalAddRef();
	}
	resource_ptr(resource_ptr& other) noexcept : ptr(other.ptr) {
		InternalAddRef();
	}
	resource_ptr(const resource_ptr& other) noexcept : ptr(other.ptr) {
		InternalAddRef();
	}


	resource_ptr(T* owner) noexcept : ptr(owner) {
		InternalAddRef();
	}


	resource_ptr(nullptr_t) noexcept : ptr(nullptr) {

	}
	resource_ptr() noexcept : ptr(nullptr) {

	}


	resource_ptr& operator= (const resource_ptr& other) {
		InternalRelease();
		ptr = other.ptr;
		InternalAddRef();
		return *this;
	}

	resource_ptr& operator=(const resource_ptr&& other) {
		InternalRelease();
		ptr = other.ptr;
		InternalAddRef();
		return *this;
	}






	~resource_ptr() {
		InternalRelease();
	}


	T* operator->() const {
		return ptr;
	}

	T* get() const {
		return ptr;
	}

	void reset() {
		InternalRelease();
	}


	bool operator==(resource_ptr<T>& other) const {
		return (ptr == other.ptr);
	}
	bool operator!=(resource_ptr<T>& other) const {
		return (ptr != other.ptr);
	}

	bool operator==(nullptr_t) const {
		return ptr == nullptr;
	}
	bool operator!=(nullptr_t) const {
		return ptr != nullptr;
	}
private:
	T* ptr;

	inline void InternalAddRef() noexcept{
		if (ptr != nullptr) {
			ptr->addref();
		}
	}

	inline void InternalRelease() noexcept{
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

	void set_err_path(const std::string& path) {
		err_path = path;

	}

	bool contains(const std::string& tag) {
		return tag_map.contains(tag);
	}

	void insert(const std::string& tag, std::unique_ptr<T> res) {
		RID rid = res->id();
		tag_map.insert_or_assign(tag, rid);
		map.insert_or_assign(rid, std::move(res));
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
			return map.at(id).get();
		}
		return nullptr;
	}

	resource_ptr<T> get_err() {
		return get(err_path);
	}

private:
	std::string err_path;
	std::unordered_map<std::string, RID> tag_map;
	std::unordered_map< RID, std::unique_ptr<T>> map;
};

