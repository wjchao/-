//*****************************************************************************
//		Copyright (C), 1993-2013, zhejiang supcon instrument Co., Ltd.
//
//		�� �� ����			TemperatureCompensation.c
//
//		��    �ܣ�			�¶Ȳ����ļ�
//      ˵    ����          �ܶȵ�λΪg/m3��kg/L
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
#include <stdio.h>
#include <stdint.h>
#include <math.h>

/**
* �����ڲ�ͷ�ļ�
*/
#include "temperatureCompensation.h"

/**
* �ڲ��궨��
*/
#define DBG_PRINT  printf

//�����������
#define     RAW_OIL          (0x10)     //ԭ��
#define     PETROL           (0x11)     //����
#define     UNDUE_OIL        (0x12)     //������
#define     SPACE_OIL        (0x13)     //��ú
#define     DIESEL_OIL       (0x14)     //����
#define     LUBE_OIL         (0x15)     //����

#define     ITER_PRECISION         (0.000001)  //VCF����ʱ�ĵ�������
#define     ITER_MAX                99        //����������


/******************************************************************************************
 * @fn           static double getBuChang(float realTemp, float stdTemp)
 * @Description  ��ò���ϵ��
 * @Parameters   float realTemp��ʵ���¶�
                 float stdTemp����׼�¶�
 * @Rerurn
 * @Notes
 * @Author       WYB
 * @Creatdate    2016-09-22
 *******************************************************************************************/
static double getComModulus(float realTemp, float stdTemp)
{
	double ret;
	double rt, st;

	rt = realTemp;
	st = stdTemp;

	ret = 1-2.3*pow(10,-5)*(rt-st)-2*pow(10,-8)*pow((rt-st),2);
    return ret;
}

/******************************************************************************************
 * @fn           static double getAlpha(uint8_t oilType, double density)
 * @Description  ���㰢����ϵ��
 * @Parameters   uint8_t oilType ����Ʒ����
                 double density���ܶ�
 * @Rerurn
 * @Notes
 * @Author       WYB
 * @Creatdate    2016-09-22
 *******************************************************************************************/
static double getAlpha(uint8_t oilType, double density)
{
	double K0,K1,A;
	double ret;

	switch(oilType)
	{
		case RAW_OIL:  //ԭ��
				K0 = 613.9723;
				K1 = 0;
				A = 0;
			break;
		case PETROL:   //����
				K0 = 346.4228;
				K1 = 0.4388;
				A = 0;
			break;
		case UNDUE_OIL:   //������
				K0 = 2680.321;
				K1 = 0;
				A = -0.00336312;
			break;
		case SPACE_OIL:   //��ú
				K0 = 594.5418;
				K1 = 0;
				A = 0;
			break;
		case DIESEL_OIL:  //����
				K0 = 186.9696;
				K1 = 0.4862;
				A = 0;
			break;
		case LUBE_OIL:    //����
				K0 = 0;
				K1 = 0.6278;
				A = 0;
			break;
		default:
				K0 = 346.4228;
				K1 = 0.4388;
				A = 0;
			break;
	}

	ret = K0/(density*density) + K1/density + A;
	return ret;

}

/******************************************************************************************
 * @fn           static double getExp(double alpha,double rou,double deltTemp)
 * @Description
 * @Parameters

 * @Rerurn
 * @Notes
 * @Author       WYB
 * @Creatdate    2016-09-22
 *******************************************************************************************/
static double getExp(double alpha,double deltTemp)
{
	double ret;

	ret = exp(-1*alpha*deltTemp*(1+0.8*alpha*deltTemp));
	return ret;
}

/** \brief
 *
 * \param oil uint8_t
 * \return uint8_t
 *
 */
static uint8_t convToCompenChart( uint8_t oil )
{
    uint8_t ret;

    switch( oil )
    {
        case CRUDE_OIL_TYPE:
        {
            ret = RAW_OIL;
            break;
        }
        case PRODUCT_OIL_TYPE:
        {
            ret = PETROL;       //��ʹ�ó�Ʒ�ͣ��ȼ������͵�15���׼�ܶȣ��ٺ�����ѡ����
            break;
        }
        case LUBE_OIL_TYPE:
        {
            ret = LUBE_OIL;
            break;
        }
        default:
        {
            ret = 0;
        }
    }

    return ret;
}

/** \brief �������͵�15���׼�ܶ�ȷ���������
 *
 * \param den15 double
 * \return uint8_t
 *
 */
static uint8_t getChartForProductOil( double den15 )
{
    uint8_t ret = 0;

    if( (den15>=838.3) && (den15<1163.5) )          //����
    {
        ret = DIESEL_OIL;
    }
    else if( (den15>=787.5) && (den15<838.3) )      //��ú
    {
        ret = SPACE_OIL;
    }
    else if( (den15>=770.3) && (den15<787.5) )      //������
    {
        ret = UNDUE_OIL;
    }
    else if( (den15>=610.6) && (den15<770.3) )      //����
    {
        ret = PETROL;
    }

    return ret;

}

/******************************************************************************************
 * @fn           static double getRealRou15(uint_8 oilType, double tstDen, double tstTemp )
 * @Description  ���������15��ʱ�ı�׼�ܶ�
 * @Parameters   uint_8 oilType����Ʒ����
                 double tstDen��ʵ���ܶȣ����ܶ�
                 double tstTemp��ʵ���¶�
 * @Rerurn
 * @Notes
 * @Author       WYB
 * @Creatdate    2016-09-22
 *******************************************************************************************/
static double getRealRou15(uint8_t oilType, double tstDen, double tstTemp )
{
	double rou15,tempRou;
	double alpha;
	double rou_t;

	uint16_t count=0;

	tempRou = tstDen;

    do{
        rou15 = tempRou;
        rou_t = tstDen*getComModulus(tstTemp, 15);
        alpha = getAlpha(oilType, rou15);
        tempRou = rou_t/getExp(alpha,(double)(tstTemp-15));
        count++;
        if(count%10==0)
        {
            #ifdef EN_WDOG
                RstWdt();
            #endif
        }
        else if(count > ITER_MAX)
        {
            DBG_PRINT("-->getRealRou15:Iterate too much! \n");
            break;
        }
        DBG_PRINT("-->getRealRou15:rou15=%lf, tempRou=%lf, alpha=%lf!\n",rou15,tempRou,alpha);
	}while(fabs(rou15-tempRou)>ITER_PRECISION);
	DBG_PRINT("-->getRealRou15:count=%3d, rou15=%f!\n",count,tempRou);
	return tempRou;
}

/******************************************************************************************
 * @fn           static double getRealRou20(uint8_t oilType, double rou15)
 * @Description  ��ȡ20����Ʒ�ܶ�
 * @Parameters   uint8_t oilType����Ʒ����
                 double rou15����Ʒ15��ı�׼�ܶ�

 * @Rerurn       20���׼�ܶȣ������ȸ�������
 * @Notes
 * @Author       WYB
 * @Creatdate    2016-09-22
 *******************************************************************************************/
static double getRealRou20(uint8_t oilType, double rou15)
{
	double rou20;
	double alpha;
	double VCF15;

	alpha = getAlpha(oilType, rou15);       //TODO�˴�Ϊ�ظ�����
	VCF15 = getExp(alpha,(20-15));
	rou20 = rou15 * VCF15;
	DBG_PRINT("-->getRealRou20:rou20=%f!\n",rou20);
	return rou20;
}

/** \brief
 *
 * \param oilType uint8_t
 * \param realTemper double
 * \param den15 double
 * \param den20 double
 * \return double
 *
 */
static double getVCF20( uint8_t type, double temper, double den15, double den20 )
{
    double ret, alpha;

    alpha = getAlpha(type, den15);    //TODO�˴�Ϊ�ظ�����
    ret = den15*getExp(alpha,(temper-15));
    ret = ret/den20;

    return ret;
}


/** \brief
 *
 * \param oilKind uint8_t
 * \param refDensity double
 * \param refTemper double
 * \param realTemper double
 * \param pRetDensity double*
 * \param pVCF20 double*
 * \return uint8_t
 *
 */
uint8_t CalCompenDensity( uint8_t oilKind, double refDensity, double refTemper, double realTemper, double * pRetDensity, double * pVCF20 )
{
    double den15;
    double retDen, retVCF20;
    uint8_t ret = 0, compenChart;

    //����Ʒ���м��
    if( (oilKind < CRUDE_OIL_TYPE) || (oilKind > LUBE_OIL_TYPE) )
    {
        return ret;
    }
    compenChart = convToCompenChart( oilKind );
    if( 0 == compenChart )  //����������
    {
        return ret;
    }

    if( PETROL == compenChart )
    {
        den15 = getRealRou15( compenChart, refDensity, refTemper );
        compenChart = getChartForProductOil( den15 );
        if( 0 == compenChart )  //��Ʒ���ܶȿռ�����
        {
            return ret;
        }
    }

    den15 = getRealRou15( compenChart, refDensity, refTemper );
    retDen = getRealRou20( compenChart, den15 );
    retVCF20 = getVCF20( compenChart, realTemper, den15, retDen );

    *pRetDensity = retDen*getComModulus( 15, 20 );  //����20�ȱ�׼�ܶ���ֵ;
    *pVCF20 = retVCF20;
    ret = 1;

    return ret;
}

