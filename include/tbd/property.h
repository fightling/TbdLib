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
  public:  void (name)(type const & _##name) { name##_=_##name; monitor(); } \
           type (name)() const { return name##_; } \
  private:

#define TBD_PROPERTY_REF_MON(type,name,monitor) \
  private: type name##_; \
  public:  void (name)(type const & _##name) { name##_=_##name; monitor(); } \
           const type& (name)() const { return name##_; } \
           type& (name)() { return name##_; } \
	private:

/// @}

/** @defgroup tomoPropertyBasic basic properties
 * @brief variable declaration including getter/setter w/o monitoring
 * @{
 */ 

#define TBD_PROPERTY_RO(type,name) \
  private: type name##_; \
  public:  type (name)() const { return name##_; } \
  private:

#define TBD_PROPERTY(type,name) \
  TBD_PROPERTY_MON(type,name,tbd::null_monitor);

#define TBD_PROPERTY_REF(type,name) \
  TBD_PROPERTY_REF_MON(type,name,tbd::null_monitor);

#define TBD_PROPERTY_PTR(type,name) \
  private: type* name##_; \
	public:  void (name)(type* _##name) { name##_=_##name; } \
          type* (name)() const { return name##_; } \
  private:

/// @}

#endif /* TBD_PROPERTY_H_ */
