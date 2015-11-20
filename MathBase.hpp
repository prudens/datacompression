#ifndef MATH_BASE_H
#define MATH_BASE_H 

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
}
#endif