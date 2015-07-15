#ifndef _XA_H_
#define _XA_H_

#include "psxcommon.h"
#include <deque>
#include "emufile.h"

struct xa_decode_t;

class EMUFILE;

struct xa_sample
{
	xa_sample() {}
	xa_sample(s16 l, s16 r)
		: left(l)
		, right(r)
	{
	}

	union {
		s16 both[2];
		struct {
			s16 left, right;
		};
	};
	s32 freq;

	void freeze(EMUFILE* fp);
	bool unfreeze(EMUFILE* fp);
};

class xa_queue : public std::deque<xa_sample>
{
public:
	xa_queue();
	void enqueue(xa_decode_t* xap);
	void advance();
	void freeze(EMUFILE* fp);
	bool unfreeze(EMUFILE* fp);
	void feed(xa_decode_t *xap);
	void fetch(s16* fourStereoSamples);
	void fetch(s32* left, s32* right);

private:
	double counter;
	double lastFrac;

	std::deque<xa_sample> curr;

};

#endif //_XA_H_
