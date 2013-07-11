#ifndef TBD_PROPERTY_H_
#define TBD_PROPERTY_H_

namespace tbd {
  inline void null_monitor() {}
}

/** @defgroup tomoPropertyManaged managed properties
 * @brief variable declaration including getter/setter/monitor
 * @{
 */ 
#define TBD_PROPERTY_MON(type,name,monitor) \
  private: type name##_; \
  public:  inline void (name)(type const & _##name) { name##_=_##name; monitor(); } \
           inline type (name)() const { return name##_; } \
  private:

#define TBD_PROPERTY_REF_MON(type,name,monitor) \
  private: type name##_; \
  public:  inline void (name)(type const & _##name) { name##_=_##name; monitor(); } \
           inline const type& (name)() const { return name##_; } \
           inline type& (name)() { return name##_; } \
	private:

/// @}

#define TBD_PROPERTY_MODIFY_FLAG() \
  protected:\
            inline void update() { modify_ = false; }\
            bool modify_; \
  public:  template<class T> bool modify(const T& _old, const T& _new ) \
           { if (_old == _new) return false;\
             modify_ = true;\
             return true; \
           } \
           inline bool modified() const { return modify_; } 

#define TBD_PROPERTY_MODIFY(type,name) \
  private: type name##_; \
  public:  inline void (name)(type const & _##name) { if( modify(name##_,_##name) ) name##_=_##name; } \
           inline type (name)() const { return name##_; } \
  private:

#define TBD_PROPERTY_REF_MODIFY(type,name) \
  private: type name##_; \
  public:  inline void (name)(type const & _##name) { if( modify(name##_,_##name) ) name##_=_##name; } \
           inline const type& (name)() const { return name##_; } \
           inline type& (name)() { modify_ = true; return name##_; } \
  private:

/** @defgroup tomoPropertyBasic basic properties
 * @brief variable declaration including getter/setter w/o monitoring
 * @{
 */ 

#define TBD_PROPERTY_RO(type,name) \
  private: type name##_; \
  public:  inline type (name)() const { return name##_; } \
  private:

#define TBD_PROPERTY_REF_RO(type,name) \
  private: type name##_; \
  public:  inline const type& (name)() const { return name##_; } \
  private:

#define TBD_PROPERTY(type,name) \
  TBD_PROPERTY_MON(type,name,tbd::null_monitor)

#define TBD_PROPERTY_REF(type,name) \
  TBD_PROPERTY_REF_MON(type,name,tbd::null_monitor)

#define TBD_PROPERTY_TYPEDEF(type,name)\
  TBD_PROPERTY(type,name)\
public:\
  typedef type name##_type;\
private:

#define TBD_PROPERTY_REF_TYPEDEF(type,name)\
  TBD_PROPERTY_REF(type,name)\
public:\
  typedef type name##_type;\
private:


/// @}

#endif /* TBD_PROPERTY_H_ */
