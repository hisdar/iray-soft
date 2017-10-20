#ifndef _BT656_MERGE_H_
#define _BT656_MERGE_H_

#define FIELD_TOP		1
#define FIELD_BOTTOM	2

#define FIELD_TYPE_TB	1  // top-bottom
#define FIELD_TYPE_OE	2  // odd-even

#define BT656_MERGE_FINISH	0
#define BT656_MERGE_HALF	1

#ifndef u32
#define u32 unsigned int
#endif

struct iray_bt656_format {
	u32 field_type;
	u32 width;
	u32 height;
	u32 bytes_per_pix;
};

class Bt656Merge {

public:
	Bt656Merge();
	~Bt656Merge();

	void getFormat(struct iray_bt656_format *fmt);
	int merge(char *src, u32 field);
	int init(struct iray_bt656_format * fmt);
	char *getData();

private:

	u32 m_is_fmt_set;
	u32 m_is_top_copied;
	u32 m_is_bottom_copied;
	
	char *m_buff;
	struct iray_bt656_format m_fmt;

	int mergeOddEven(char * src, u32 field);
	int mergeTopBottom(char *src, u32 field);
};

#endif
