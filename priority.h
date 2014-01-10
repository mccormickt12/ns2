/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 1996-1997 The Regents of the University of California.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 * 	This product includes software developed by the Network Research
 * 	Group at Lawrence Berkeley National Laboratory.
 * 4. Neither the name of the University nor of the Laboratory may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * @(#) $Header: /cvsroot/nsnam/ns-2/Priority/Priority.h,v 1.35 2005/01/13 18:33:48 haldar Exp $ (LBL)
 */

#ifndef ns_priority_h
#define ns_priority_h

#include "connector.h"
#include "packet.h"
#include "ip.h"
class Packet;

class PacketPriority : public TclObject {
public:
	PacketPriority() : head_(0), tail_(0), len_(0), bytes_(0) {}
	virtual void place(Packet *p, int qos)
	{
	    Packet *point;
	    Packet *last;
	    hdr_ip*   iph;
	    point = last = head_;
	    while (true) 
	    {
	      iph = hdr_ip::access(point);
	      int point_qos = int(iph->prio_);
	      printf("Packet priority is %d", point_qos);
	      if (qos < point_qos)
	      {
	          if (point == last) 
	          {
	              p->next_ = last;
	              point = p;
	          } else {
	              p->next_ = point;
	              last->next_ = p;
	          }
	          return;
	      } else if (point == 0)
	      {
	          point->next_ = p;
	          p->next_ = 0;
	      } else 
	      {
	          last = point;
	          point = point->next_;
	      }
	    }
	}

	virtual int length() const { return (len_); }
	virtual int byteLength() const { return (bytes_); }
	virtual Packet* enque(Packet* p) { // Returns previous tail
		printf("here");
		Packet* pt = tail_;
		if (!tail_) head_= tail_= p;
		hdr_ip*   iph = hdr_ip::access(p);
    	int qos = int(iph->prio_);
    	place(p, qos);
    	++len_;
		bytes_ += hdr_cmn::access(p)->size();
		return pt;
	}
	virtual Packet* deque() {
		if (!head_) return 0;
		Packet* p = head_;
		head_= p->next_; // 0 if p == tail_
		if (p == tail_) head_= tail_= 0;
		--len_;
		bytes_ -= hdr_cmn::access(p)->size();
		return p;
	}
	Packet* lookup(int n) {
		for (Packet* p = head_; p != 0; p = p->next_) {
			if (--n < 0)
				return (p);
		}
		return (0);
	}
	/* remove a specific packet, which must be in the Priority */
	virtual void remove(Packet*);
	/* Remove a packet, located after a given packet. Either could be 0. */
	void remove(Packet *, Packet *);
        Packet* head() { return head_; }
	Packet* tail() { return tail_; }
	// MONARCH EXTNS
	virtual inline void enqueHead(Packet* p) {
	        if (!head_) tail_ = p;
	        p->next_ = head_;
		head_ = p;
		++len_;
		bytes_ += hdr_cmn::access(p)->size();
	}
        void resetIterator() {iter = head_;}
        Packet* getNext() { 
	        if (!iter) return 0;
		Packet *tmp = iter; iter = iter->next_;
		return tmp;
	}

protected:
	Packet* head_;
	Packet* tail_;
	int len_;		// packet count
	int bytes_;		// Priority size in bytes


// MONARCH EXTNS
private:
	Packet *iter;
};

class Priority;

class PriorityHandler : public Handler {
public:
	inline PriorityHandler(Priority& q) : Priority_(q) {}
	void handle(Event*);
private:
	Priority& Priority_;
};


class Priority : public Connector {
public:
	virtual void enque(Packet*) = 0;
	virtual Packet* deque() = 0;
	virtual void recv(Packet*, Handler*);
	virtual void updateStats(int Prioritysize); 
	void resume();
	
	int blocked() const { return (blocked_ == 1); }
	void unblock() { blocked_ = 0; }
	void block() { blocked_ = 1; }
	int limit() { return qlim_; }
	int length() { return pq_->length(); }	/* number of pkts currently in
						 * underlying packet Priority */
	int byteLength() { return pq_->byteLength(); }	/* number of bytes *
						 * currently in packet Priority */
	/* mean utilization, decaying based on util_weight */
	virtual double utilization (void);

	/* max utilization over recent time period.
	   Returns the maximum of recent measurements stored in util_buf_*/
	double peak_utilization(void);
	virtual ~Priority();
protected:
	Priority();
	void reset();
	int qlim_;		/* maximum allowed pkts in Priority */
	int blocked_;		/* blocked now? */
	int unblock_on_resume_;	/* unblock q on idle? */
	PriorityHandler qh_;
	PacketPriority *pq_;	/* pointer to actual packet Priority 
				 * (maintained by the individual disciplines
				 * like DropTail and RED). */
	double true_ave_;	/* true long-term average Priority size */
	double total_time_;	/* total time average Priority size compute for */


	void utilUpdate(double int_begin, double int_end, int link_state);
	double last_change_;  /* time at which state changed/utilization measured */
	double old_util_;     /* current utilization */ 
	double util_weight_;  /* decay factor for measuring the link utilization */
	double util_check_intv_; /* interval for reseting the current
				    utilization measurements (seconds) */
	double period_begin_;	/* time of starting the current utilization
				   measurement */
	double cur_util_;	/* utilization during current time period */
	int buf_slot_;		/* Currently active utilization buffer */
	double *util_buf_;    /* Buffer for recent utilization measurements */
	int util_records_;	/* Number of recent utilization measurements
				   stored in memory. One slot in buffer holds
				   period of util_check_intv_ seconds. */
	// measuring #drops
	
};

#endif
