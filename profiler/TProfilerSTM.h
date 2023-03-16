#ifndef _H_STM32_PROFILER_H_
#define _H_STM32_PROFILER_H_



typedef struct {
	unsigned long cur_ticks;
	unsigned long min_time;
	unsigned long max_time;
} S_TAGPRFLR_T;

#define C_PROFLR_POINT_AMOUNT 10



class TPROFILER {
		S_TAGPRFLR_T point[C_PROFLR_POINT_AMOUNT];
		unsigned long get_rawticks ();
		unsigned long get_proticker ();
		
	public:
		TPROFILER ();
		void Init ();
		void Clear ();
		void Clear_Statistics (long pnt_ix);
		void Start_Point (long pnt_ix);
		void Stop_Point (long pnt_ix);
};



#endif
