
class MyEnum : public EnumSerializer<MyEnum>
{
  TBD_ENUM
  (
    none,
    speed,
    overlap,
    material_amount,
    travel
  )

  MyEnum(size_t _value)

private:
  size_t value_;


};
