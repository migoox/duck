#pragma once
namespace mini::gk2
{
class Simulation
{
  public:
    Simulation();
    virtual ~Simulation() = default;

    bool Update(double dt);

    double StepTime() const
    {
        return m_stepTime;
    }

    double SimSpeed() const
    {
        return m_simSpeed;
    }

    void SetSimSpeed(double stepTime)
    {
        m_simSpeed = stepTime;
    }

  protected:
    virtual void PostUpdate() {};
    virtual void Step() = 0;

    double DeltaTime() const
    {
        return m_deltaTime;
    }

    void SetStepTime(double stepTime)
    {
        m_stepTime = stepTime;
    }

  private:
    double m_stepTime;
    double m_simSpeed;
    double m_deltaTime;
};
} // namespace mini::gk2
