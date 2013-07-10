#ifndef __CHudMessageQueue_H__
#define __CHudMessageQueue_H__

class CHudMessageQueue
{
public:
	CHudMessageQueue(int nTopLeftCornerX, int nTopLeftCornerY, u_int nMessageLimit, float fMessageTimeout);
	~CHudMessageQueue(void);

	void AddMessage(string sMessage);
	void Render(void);

private:
	list<string>	m_oMessages;
	float			m_fMessageTimer;

	int				m_nTopLeftCornerX;
	int				m_nTopLeftCornerY;
	u_int			m_nMessageLimit;
	float			m_fMessageTimeout;

	GLFWmutex		m_oMessageMutex;

	static const float	m_kfMessageTimeoutThreshold;
	static const int	m_knHorizontalDistance;
};

#endif // __CHudMessageQueue_H__
