#ifndef DECLARE_PROPERTY_H_
#define DECLARE_PROPERTY_H_

namespace tbd
{

#define TBD_DECLARE_PROPERTY_RO(type,name) \
	private:  type                                            name##_; \
	public:   type (name)() const                      { return name##_; } \
	private:

#define TBD_DECLARE_PROPERTY(type,name) \
	private: 	type 						name##_; \
	public:   	void  (name)(type _##name)  { name##_=_##name; } \
				type (name)() const        { return name##_; } \
	private:

#define TBD_DECLARE_PROPERTY_REF(type,name) \
	private: 	type 						name##_; \
	public:   	void  (name)(type& _##name)  { name##_=_##name; } \
				type& (name)()        { return name##_; } \
	private:

#define TBD_DECLARE_PROPERTY_PTR(type,name) \
	private: 	type* 						name##_; \
	public:   	void  (name)(type* _##name)  { name##_=_##name; } \
				type* (name)()  const   { return name##_; } \
	private:
}

#endif /* DECLARE_PROPERTY_H_ */
