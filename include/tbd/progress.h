/*
 * progress.h
 *
 *  Created on: 17 Oct 2011
 *      Author: pat
 */

#ifndef PROGRESS_H_
#define PROGRESS_H_

namespace tbd
{
  struct Progress
  {
    Progress()
        : count_(100.0),
          step_(1.0)
    {
    }
  protected:
    Progress(double _count, double _step = 1.0)
        : count_(_count),
          step_(_step)
    {
    }
  public:
    double next()
    {
      value_ += step_;
      if (value_ > count_)
        value_ = count_;
      update();
      return value_;
    }
    virtual void update() = 0;
    virtual void state( const std::string& _state ) = 0;
    double count_, step_, value_;
  };

  struct ConsoleProgress: Progress
  {
    ConsoleProgress(std::ostream& os = std::cout, double _count = 100.0, double _step = 1.0)
        : Progress(_count, _step),
          os_(os)
    {
    }
    virtual void state( const std::string& _state )
    {
      os_ << _state << std::endl;
    }
    virtual void update()
    {
      os_ << value_ << std::endl;
    }
  private:
    std::ostream& os_;
  };

  struct NulProgress: Progress
  {
    NulProgress(double _count = 100.0, double _step = 1.0)
        : Progress(_count, _step)
    {
    }
    virtual void update()
    {
    }
    virtual void state( const std::string& _state )
    {
    }
  };

  struct ProgressPass: Progress
  {
    ProgressPass(double _length, Progress& _progress, double _count = 100.0, double _step = 1.0)
        : Progress(_count, _step),
          length_(_length),
          progress_(_progress),
          start_(progress_.value_)
    {
    }
    ProgressPass(const std::string& _name, double _length, Progress& _progress, double _count = 100.0, double _step = 1.0)
        : Progress(_count, _step),
          length_(_length),
          progress_(_progress),
          start_(progress_.value_),
          name_(_name)
    {
      state("");
    }
    virtual void update()
    {
      progress_.value_ = value_ / count_ * length_ + start_;
      progress_.update();
    }
    virtual void state( const std::string& _state )
    {
      progress_.state(name_ + _state);
    }
  private:
    double length_;
    Progress& progress_;
    double start_;
    std::string name_;
  };
}

#endif /* PROGRESS_H_ */
