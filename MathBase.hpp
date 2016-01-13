#ifndef MATH_BASE_H
#define MATH_BASE_H 
#include "common.h"
#define PI 3.141592654 //pi的值
//Information Theory
//i(A) = -(log(P(A))/log(b) // base b
double SelfInformation( double pEvent,double base )
{
    double ret = log( pEvent );
    return -log( pEvent ) / log( base );
}
// 平均自由量，即信息的熵
double MeanSelfInfo( double* pEventList, size_t  size ,double base )
{
    double result = 0;
    for ( size_t i = 0; i < size; i++ )
    {
        result += ( pEventList[i] * SelfInformation(pEventList[i],base) );
    }
    return result;
}


// 直角坐标与极坐标的转换
typedef std::pair < double,double > rect_coord;
typedef std::pair<double, double> pol_coord;

pol_coord RectCoord2PolCoord(const rect_coord& rc)
{
    pol_coord pc;
    pc.first = sqrt( rc.first*rc.first + rc.second*rc.second );
    pc.second = atan(rc.second/rc.first);
    return pc;
}
rect_coord PolCoord2RectCoord( const pol_coord& pc )
{
    rect_coord rc;
    rc.first = pc.first*cos(pc.second);
    rc.second = pc.first*sin( pc.second );
    return rc;
}

void test_mathbase( int argc, char**argv )
{
    std::cout << "Hello,Wolrd!!!" << std::endl;
    double pEvent = 1;
    double pEventlist[3] = { 0.25, 0.25, 0.5 };
    for ( int i = 0; i < 100; i++, pEvent -= 0.01 )
    {
        // pEventlist[i] = pEvent;
        std::cout << "if event probability is " << pEvent << "  then self information is : " << SelfInformation( pEvent, 2 ) << std::endl;

    }
    std::cout << "mean self infromation " << MeanSelfInfo( pEventlist, sizeof( pEventlist ) / sizeof( pEventlist[0] ), 2 ) << std::endl;

    BEGIN_TEST( RectCoord2PolCoord );
    rect_coord rc(1,1);
    pol_coord pc = RectCoord2PolCoord( rc );
    std::cout << "the input rect_coord is (" << rc.first << " ," << rc.second << "), the output pol coord is ( " << pc.first << " ," << pc.second << ")\n";
    END_TEST( RectCoord2PolCoord );

    BEGIN_TEST( PolCoord2RectCoord );
    rc = PolCoord2RectCoord( pc );
    std::cout << "the input polar coord is (" << pc.first << " ," << pc.second << "), the output rect coord is ( " << rc.first << " ," << rc.second << ")\n";
    END_TEST( PolCoord2RectCoord );
 }
#endif