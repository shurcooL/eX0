#ifndef __CHudMessageQueue_H__
#define __CHudMessageQueue_H__

class CHudMessageQueue
{
public:
	CHudMessageQueue(int nTopLeftCornerX, int nTopLeftCornerY, u_int nMessageLimit, double dMessageTimeout);
	~CHudMessageQueue(void);

	void AddMessage(string sMessage);
	void Render(void);

private:
	std::list<string>	m_oMessages;
	double				m_dMessageTimer;

	int					m_nTopLeftCornerX;
	int					m_nTopLeftCornerY;
	u_int				m_nMessageLimit;
	double				m_dMessageTimeout;

	GLFWmutex			m_oMessageMutex;

	static const double	m_kdMessageTimeoutThreshold;
	static const int	m_knHorizontalDistance;
};

#endif // __CHudMessageQueue_H__
