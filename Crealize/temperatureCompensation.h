//*****************************************************************************
//		Copyright (C), 1993-2013, zhejiang supcon instrument Co., Ltd.
//
//		�� �� ����			TemperatureCompensation.h
//
//		��    �ܣ�			�¶Ȳ���ͷ�ļ�
//
//		��    ����			V1.00�b
//		��    �ߣ�		    �⾲��
//		�������ڣ�			2017.10.26
//
//		�޸Ĵ�����
//		�޸�ʱ�䣺
//		�޸����ݣ�
//		�� �� �ߣ�
//*****************************************************************************
/**
* ������ͷ�ļ�
*/
#include <stdint.h>

//�������ͷ���
#define CRUDE_OIL_TYPE      0   //ԭ��
#define PRODUCT_OIL_TYPE    1   //��Ʒ��
#define LUBE_OIL_TYPE       2   //����

/**
*   ���⿪�Žӿ�
*/
//���㲹������ܶ�ֵ
uint8_t CalCompenDensity( uint8_t oilKind, double refDensity, double refTemper, double realTemper, double * pRetDensity, double * pVCF20 );

//test
//double getRealRou15(uint8_t oilType, float tstDen, float tstTemp );
