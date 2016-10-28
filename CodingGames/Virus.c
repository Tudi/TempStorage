#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/
 
int NodeList[500][2];
int GateWayList[500];
int GateWayLinks[500];
int N; // the total number of nodes in the level, including the gateways
int L; // the number of links
int E; // the number of exit gateways

int IsGatewayNode( int n )
{
    for( int i=0;i<E;i++)
        if( GateWayList[i] == n )
            return 1;
    return 0;
}

int IsGateWayLink( int n )
{
    for( int i=0;i<L;i++)
    {
        if( NodeList[i][0] == n || NodeList[i][1] == n )
        {
//fprintf(stderr, "agent node %d is on link %d %d\n",n, NodeList[i][0], NodeList[i][1] );            
            if( IsGatewayNode( NodeList[i][0] ) )
            {
//fprintf(stderr, "agent node %d is linked to a gateway node %d\n",n, NodeList[i][0]);            
                return i;
            }
            if( IsGatewayNode( NodeList[i][1] ) )
            {
//fprintf(stderr, "agent node %d is linked to a gateway node %d\n",n, NodeList[i][1]);            
                return i;
            }
        }
    }
//fprintf(stderr, "agent is not on a gateway link\n");    
    return -1;
}

int GetGWInd( int n )
{
    for( int i=0;i<E;i++)
        if( GateWayList[i] == n )
            return i;
    return -1;
}

int CountGateWayLinks( int ind, int gw )
{
    if( ind < 0 )
        ind = GetGWInd( gw );
    GateWayLinks[ ind ] = 0;
    for( int i=0;i<L;i++)
    {
        if( NodeList[i][0] == gw || NodeList[i][1] == gw )
            if( NodeList[i][0] >= 0 && NodeList[i][1] >= 0 )
                GateWayLinks[ ind ]++;
    }
}

int GetMostConnectedGW()
{
    int BestConnectionCount = 0;
    int BestIndex = 0;
    for (int i = 0; i < E; i++) 
        if( GateWayLinks[i] > BestConnectionCount )
//        if( GateWayLinks[i] < BestConnectionCount )
        {
            BestConnectionCount = GateWayLinks[i];
            BestIndex = i;
        }
    return GateWayList[ BestIndex ];
}

void CutLinkGW( int gw )
{
    for( int i=0;i<L;i++)
    {
        if( NodeList[i][0] == gw || NodeList[i][1] == gw )
            if( NodeList[i][0] >= 0 && NodeList[i][1] >= 0 )
            {
                printf("%d %d\n", NodeList[i][0], NodeList[i][1] );
                if( NodeList[i][0] != gw )
                    NodeList[i][0] = -1;
                if( NodeList[i][1] != gw )
                    NodeList[i][1] = -1;
                break;
            }
    }
    CountGateWayLinks( -1, gw );
}

void PrintLinks()
{
    for( int i=0;i<L;i++)
        fprintf(stderr, "link %d %d\n",NodeList[i][0], NodeList[i][1] );         
}

int main()
{
    scanf("%d%d%d", &N, &L, &E);
    for (int i = 0; i < L; i++) {
        int N1; // N1 and N2 defines a link between these nodes
        int N2;
        scanf("%d%d", &N1, &N2);
        NodeList[i][0] = N1;
        NodeList[i][1] = N2;
    }
    for (int i = 0; i < E; i++) {
        int EI; // the index of a gateway node
        scanf("%d", &EI);
        GateWayList[i] = EI;
        CountGateWayLinks( i, EI );
fprintf(stderr, "gateway is at %d Connections %d\n", EI, GateWayLinks[i] );        
    }

    // game loop
    
    while (1) {
        int SI; // The index of the node on which the Skynet agent is positioned this turn
        scanf("%d", &SI);
fprintf(stderr, "agent is at %d\n", SI );
//printf("%d %d\n", NodeList[0][0], NodeList[0][1]);
        // Write an action using printf(). DON'T FORGET THE TRAILING \n
        // To debug: fprintf(stderr, "Debug messages...\n");
//        PrintLinks();
        //is the agent near a gateway node ?
        int BestLink = IsGateWayLink( SI );
//fprintf(stderr, "best link for node %d is %d \n", SI, BestLink );        
        if( BestLink >= 0 )
        {
            printf("%d %d\n", NodeList[BestLink][0], NodeList[BestLink][1] );
            if( IsGatewayNode( NodeList[BestLink][0] ) == 0 )
                NodeList[BestLink][0] = -1;
            else
                CountGateWayLinks( -1, NodeList[BestLink][0] );
            if( IsGatewayNode( NodeList[BestLink][1] ) == 0 )
                NodeList[BestLink][1] = -1;
            else
                CountGateWayLinks( -1, NodeList[BestLink][1] );
        }
        else
        {
            int DangerGW = GetMostConnectedGW();
            fprintf(stderr, "most connected GW is %d\n",DangerGW);
            CutLinkGW( DangerGW );
        } /**/
/*        if( SI == 11 )
            printf("0 9\n");
        if( SI == 5 )
            printf("0 5\n");
            */
            
        //severe one of the gateways. Least links or most links
        
        //what is easier ? Sever the links near the agent or sever the links of the gateways ?

//        printf("0 1\n"); // Example: 0 1 are the indices of the nodes you wish to sever the link between
    }

    return 0;
}