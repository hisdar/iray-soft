#include <memory.h>
#include <malloc.h>
#include "Bt656Merge.h"

Bt656Merge::Bt656Merge()
{
	m_is_fmt_set = 0;
	m_buff = NULL;
}

Bt656Merge::~Bt656Merge()
{
	free(m_buff);
}

int Bt656Merge::init(struct iray_bt656_format *fmt)
{
	memcpy(&m_fmt, fmt, sizeof(struct iray_bt656_format));
	
	if (m_buff != NULL) {
		free(m_buff);
		m_buff = NULL;
	}

	m_buff = (char *)malloc(fmt->height * fmt->width * fmt->bytes_per_pix);
	if (m_buff == NULL) {
		return -1;
	}

	m_is_fmt_set = 1;
	m_is_bottom_copied = 0;
	m_is_top_copied = 0;

	return 0;
}

int Bt656Merge::mergeOddEven(char *src, u32 field)
{
	u32 line_len = m_fmt.bytes_per_pix * m_fmt.width;
	char *tag_addr = m_buff;
	char *src_addr = src;
	
	if (field == FIELD_TOP) {
		if (m_is_top_copied) {
			printf("Warning: a top frame will be cover(drop)\n");
		}
		
		m_is_top_copied = 1;
	} else if (field == FIELD_BOTTOM) {
		if (m_is_bottom_copied == 1) {
			printf("Warning: a bottom frame will be cover(drop)\n");
		}
		tag_addr += line_len;
		m_is_bottom_copied = 1;
	}

	// copy data
	for (u32 i = 0; i < m_fmt.height / 2; i++)
	{
		memcpy(tag_addr, src_addr, line_len);
		tag_addr += line_len * 2;
		src_addr += line_len;
	}

	if (m_is_bottom_copied && m_is_top_copied) {
		m_is_bottom_copied = 0;
		m_is_top_copied = 0;
		return BT656_MERGE_FINISH;
	}

	return BT656_MERGE_HALF;
}

int Bt656Merge::mergeTopBottom(char *src, u32 field)
{
	u32 line_len = m_fmt.bytes_per_pix * m_fmt.width;
	char *tag_addr = m_buff;
	char *src_addr = src;

	printf("merge top bottom\n");	
	if (field == FIELD_TOP) {
		if (m_is_top_copied) {
			printf("Warning: a top frame will be cover(drop)\n");
		}
		printf("merge top\n");			
		m_is_top_copied = 1;
	} else if (field == FIELD_BOTTOM) {
		if (m_is_bottom_copied == 1) {
			printf("Warning: a bottom frame will be cover(drop)\n");
		}
		
		tag_addr += (line_len * m_fmt.height / 2);
		m_is_bottom_copied = 1;
	}
	
	// copy data
	for (u32 i = 0; i < m_fmt.height / 2; i++)
	{
		memcpy(tag_addr, src_addr, line_len);
		tag_addr += line_len;
		src_addr += line_len;
	}

	if (m_is_bottom_copied && m_is_top_copied) {
		m_is_bottom_copied = 0;
		m_is_top_copied = 0;
		return BT656_MERGE_FINISH;
	}

	return BT656_MERGE_HALF;
}

int Bt656Merge::merge(char *src, u32 field)
{
	if (!m_is_fmt_set) {
		printf("Error, should call init function befor merge\n");
		return -1;
	}
	
	if (m_fmt.field_type == FIELD_TYPE_OE) {
		return mergeOddEven(src, field);
	} else if (m_fmt.field_type == FIELD_TYPE_TB) {
		return mergeTopBottom(src, field);
	}

	return -1;
}

void Bt656Merge::getFormat(struct iray_bt656_format *fmt)
{
	memcpy(fmt, &m_fmt, sizeof(struct iray_bt656_format));
}

char *Bt656Merge::getData()
{
	return m_buff;
}


