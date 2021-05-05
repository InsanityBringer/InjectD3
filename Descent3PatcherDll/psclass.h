/*
 THE COMPUTER CODE CONTAINED HEREIN IS THE SOLE PROPERTY OF OUTRAGE
 ENTERTAINMENT, INC. ("OUTRAGE").  OUTRAGE, IN DISTRIBUTING THE CODE TO
 END-USERS, AND SUBJECT TO ALL OF THE TERMS AND CONDITIONS HEREIN, GRANTS A
 ROYALTY-FREE, PERPETUAL LICENSE TO SUCH END-USERS FOR USE BY SUCH END-USERS
 IN USING, DISPLAYING,  AND CREATING DERIVATIVE WORKS THEREOF, SO LONG AS
 SUCH USE, DISPLAY OR CREATION IS FOR NON-COMMERCIAL, ROYALTY OR REVENUE
 FREE PURPOSES.  IN NO EVENT SHALL THE END-USER USE THE COMPUTER CODE
 CONTAINED HEREIN FOR REVENUE-BEARING PURPOSES.  THE END-USER UNDERSTANDS
 AND AGREES TO THE TERMS HEREIN AND ACCEPTS THE SAME BY USE OF THIS FILE.
 COPYRIGHT 1996-2000 OUTRAGE ENTERTAINMENT, INC.  ALL RIGHTS RESERVED.
 */

#pragma once

 //	tQueue 
 //		a circular queue implementation

template <class T, int t_LEN> class tQueue
{
	T m_items[t_LEN];
	short m_head, m_tail;

public:
	tQueue() { m_head = m_tail = 0; };
	~tQueue() { };

	// sends an item onto the queue
	void send(T& item) 
	{
		short temp = m_tail + 1;
		if (temp == t_LEN)
			temp = 0;
		if (temp != m_head) 
		{
			m_items[m_tail] = item;
			m_tail = temp;
		}
	};
	// returns an item from the queue, false if no item.
	bool recv(T* item) 
	{
		if (m_head == m_tail)
			return false;
		*item = m_items[m_head++];
		if (m_head == t_LEN)
			m_head = 0;
		return true;
	};

	// flush queue entries.
	void flush() 
	{
		m_head = m_tail = 0;
	};
};
