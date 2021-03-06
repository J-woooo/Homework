#include <stdio.h>
#define _CRT_SECURE_NO_WARNINGS
/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for PA2, unidirectional or bidirectional
   data transfer protocols (from A to B. Bidirectional transfer of data
   is for extra credit and is not required).  Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/

#define BIDIRECTIONAL 0    /* change to 1 if you're doing extra credit */
/* and write a routine called B_output */

/* a "msg" is the data unit passed from layer 5 (teachers code) to layer  */
/* 4 (students' code).  It contains the data (characters) to be delivered */
/* to layer 5 via the students transport level protocol entities.         */
struct msg {    //전송되고자 하는 메세지
    char data[20];
};

/* a packet is the data unit passed from layer 4 (students code) to layer */
/* 3 (teachers code).  Note the pre-defined packet structure, which all   */
/* students must follow. */
struct pkt {    //패킷
    int seqnum;
    int acknum;
    int checksum;
    char payload[20];
};

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/

struct pkt send_pkt[1010];
struct pkt rcv_pkt[1010];
int pkt_no;
int nxt_seq_no;
static int s_base;
int r_base;
float diff;
int buffer_msg;
int buffer_send;
int seq_no;
int flag;
int N;
float RTT;

struct pkt_timer
{
    int seqnum;
    int acknum;
    float start_time;
    float rtt;
};
struct pkt_timer timeout[1000];

int checksum_call(int seq_no, int ack_no, char* payload)
{
    int sum = 0;
    sum = seq_no + ack_no;
    int i;
    for (i = 0; i < 20; i++) {
        sum += payload[i];
    }
    //printf("\nSum = %d\n",sum);
    return sum;
}

/* called from layer 5, passed the data to be sent to other side */
A_output(message)
struct msg message;
{
    printf("In A_output\n");
    if (nxt_seq_no < s_base + N)
    {
        int i;
        for (i = 0; i < 20; i++)
            send_pkt[pkt_no].payload[i] = message.data[i];
        send_pkt[pkt_no].seqnum = seq_no;
        send_pkt[pkt_no].acknum = 0;
        send_pkt[pkt_no].checksum = checksum_call(send_pkt[pkt_no].seqnum, send_pkt[pkt_no].acknum, send_pkt[pkt_no].payload);
        tolayer3(0, send_pkt[pkt_no]);
        timeout[pkt_no].seqnum = seq_no;
        timeout[pkt_no].acknum = 0;
        timeout[pkt_no].start_time = get_sim_time();
        timeout[pkt_no].rtt = get_sim_time() + RTT;
        printf("rtt time %f", timeout[pkt_no].rtt);
        if (s_base == nxt_seq_no)
        {
            starttimer(0, RTT);
        }
        seq_no++;
        nxt_seq_no++;
        pkt_no++;
        return;
    }
    else
    {   //out-of-order packet을 buffer에 보관
        int i;
        for (i = 0; i < 20; i++)
            send_pkt[pkt_no].payload[i] = message.data[i];
        send_pkt[pkt_no].seqnum = seq_no;
        send_pkt[pkt_no].acknum = 0;
        send_pkt[pkt_no].checksum = checksum_call(send_pkt[pkt_no].seqnum, send_pkt[pkt_no].acknum, send_pkt[pkt_no].payload);
        buffer_msg++;
        seq_no++;
        pkt_no++;
        return;
    }
}

B_output(message)  /* need be completed only for extra credit */
struct msg message;
{
}

/* called from layer 3, when a packet arrives for layer 4 */
A_input(packet)
struct pkt packet;
{
    printf("In A_input\n");
    int ack_checksum = 0;
    ack_checksum = checksum_call(packet.seqnum, packet.acknum, packet.payload);
    if (packet.acknum == -1)
    {
        return;
    }
    if (ack_checksum == packet.checksum)
    {
        send_pkt[packet.acknum].acknum = 1;
        timeout[packet.acknum].acknum = 1;
        if (s_base == packet.acknum)
        {
            int i;
            i = packet.acknum;
            while (send_pkt[i].acknum == 1)
            {
                printf("incrementing s_base");
                i++;
                s_base++;
            }
        }

        if (s_base == nxt_seq_no)
        {
            stoptimer(0);
        }
        buffer_send = nxt_seq_no;
        while (buffer_msg != 0 && nxt_seq_no < s_base + N)
        {

            tolayer3(0, send_pkt[buffer_send]);
            timeout[buffer_send].seqnum = seq_no;
            timeout[buffer_send].acknum = 0;
            timeout[pkt_no].start_time = get_sim_time();
            timeout[buffer_send].rtt = get_sim_time() + RTT;
            if (s_base == nxt_seq_no)
            {
                starttimer(0, RTT);
            }
            nxt_seq_no++;
            buffer_send++;
            buffer_msg--;
        }

    }
    else if (ack_checksum != packet.checksum)
    {
        printf("Corrupted ack\n");
    }
    return;
}

/* called when A's timer goes off */
A_timerinterrupt()
{
    printf("In A_timerinterrupt\n");
    int num, i;
    num = nxt_seq_no - s_base;
    int pkt = s_base;
    float currenttime;
    currenttime = get_sim_time();
    starttimer(0, diff);
    printf("%f vsvsdnvoidsvdsovndsvsdonvbsdvsoivni", diff);

    for (i = 0; i < num; i++)
    {
        if (send_pkt[pkt].acknum == 0 && (currenttime > timeout[pkt].rtt))
        {
            tolayer3(0, send_pkt[pkt]);
            timeout[pkt].seqnum = pkt;
            timeout[pkt].acknum = 0;
            timeout[pkt_no].start_time = get_sim_time();
            timeout[pkt].rtt = get_sim_time() + RTT;
            break;
        }
        pkt++;
    }
}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
A_init()
{
    N = getwinsize();
    RTT = 32;
    pkt_no = 0;
    nxt_seq_no = 0;
    s_base = 0;
    diff = 2.0;
    buffer_msg = 0;
    seq_no = 0;
    buffer_send = 0;
}


/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
B_input(packet)
struct pkt packet;
{
    printf("In B_input \n");
    int pkt_checksum = 0;
    pkt_checksum = checksum_call(packet.seqnum, packet.acknum, packet.payload);
    int seq = packet.seqnum;
    if (pkt_checksum == packet.checksum && r_base == packet.seqnum)
    {
        rcv_pkt[r_base] = packet;
        int i;
        for (i = 0; i < 20; i++)
        {
            rcv_pkt[packet.seqnum].payload[i] = packet.payload[i];
        }
        if (flag)
        {
            tolayer5(1, rcv_pkt[r_base].payload);
            r_base++;
            flag = 0;
            while (seq = rcv_pkt[r_base].seqnum)
            {
                tolayer5(1, rcv_pkt[r_base].payload);
                r_base++;
            }
        }
        while (seq > 0)
        {
            printf("delivering data to upper layer\n");
            tolayer5(1, rcv_pkt[r_base].payload);
            r_base++;
            seq = rcv_pkt[r_base].seqnum;
        }
        struct pkt ackpkt = packet;
        ackpkt.acknum = packet.seqnum;
        ackpkt.checksum = checksum_call(ackpkt.seqnum, ackpkt.acknum, ackpkt.payload);
        tolayer3(1, ackpkt);
    }
    else if (pkt_checksum == packet.checksum && (packet.seqnum < r_base + N))
    {
        rcv_pkt[packet.seqnum] = packet;
        int i;
        for (i = 0; i < 20; i++)
        {
            rcv_pkt[packet.seqnum].payload[i] = packet.payload[i];
        }
        struct pkt ackpkt = packet;
        ackpkt.acknum = packet.seqnum;
        ackpkt.checksum = checksum_call(ackpkt.seqnum, ackpkt.acknum, ackpkt.payload);
        tolayer3(1, ackpkt);
    }
    else
    {
        printf("Corrupted packet or undesired packet\n");

    }
    return;
}

/* called when B's timer goes off */
B_timerinterrupt()  //B의 timeout은 고려할 필요 없으므로 공란
{

}
/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
B_init()
{
    r_base = 0;
    flag = 1;
}


/*****************************************************************
***************** NETWORK EMULATION CODE STARTS BELOW ***********
The code below emulates the layer 3 and below network environment:
  - emulates the tranmission and delivery (possibly with bit-level corruption
    and packet loss) of packets across the layer 3/4 interface
  - handles the starting/stopping of a timer, and generates timer
    interrupts (resulting in calling students timer handler).
  - generates message to be sent (passed from later 5 to 4)

THERE IS NOT REASON THAT ANY STUDENT SHOULD HAVE TO READ OR UNDERSTAND
THE CODE BELOW.  YOU SHOLD NOT TOUCH, OR REFERENCE (in your code) ANY
OF THE DATA STRUCTURES BELOW.  If you're interested in how I designed
the emulator, you're welcome to look at the code - but again, you should have
to, and you defeinitely should not have to modify
******************************************************************/

struct event {
    float evtime;           /* event time */
    int evtype;             /* event type code */
    int eventity;           /* entity where event occurs */
    struct pkt* pktptr;     /* ptr to packet (if any) assoc w/ this event */
    struct event* prev;
    struct event* next;
};
struct event* evlist = NULL;   /* the event list */

/* possible events: */
#define  TIMER_INTERRUPT 0  
#define  FROM_LAYER5     1
#define  FROM_LAYER3     2

#define  OFF             0
#define  ON              1
#define   A    0
#define   B    1



int TRACE = 1;             /* for my debugging */
int nsim = 0;              /* number of messages from 5 to 4 so far */
int nsimmax = 0;           /* number of msgs to generate, then stop */
float time = 0.000;
float lossprob;            /* probability that a packet is dropped  */
//float corruptprob;         /* probability that one bit is packet is flipped */
//패킷이 손실될 경우는 고려하지 않으므로 배제한다.
int windowSize;
int RTT;
float lambda;              /* arrival rate of messages from layer 5 */
int   ntolayer3;           /* number sent into layer 3 */
int   nlost;               /* number lost in media */
int ncorrupt;              /* number corrupted by media*/

main()
{
    struct event* eventptr;
    struct msg  msg2give;
    struct pkt  pkt2give;

    int i, j;
    char c;

    init();
    A_init();
    B_init();

    while (1) {
        eventptr = evlist;            /* get next event to simulate */
        if (eventptr == NULL)
            goto terminate;
        evlist = evlist->next;        /* remove this event from event list */
        if (evlist != NULL)
            evlist->prev = NULL;
        if (TRACE >= 2) {
            printf("\nEVENT time: %f,", eventptr->evtime);
            printf("  type: %d", eventptr->evtype);
            if (eventptr->evtype == 0)
                printf(", timerinterrupt  ");
            else if (eventptr->evtype == 1)
                printf(", fromlayer5 ");
            else
                printf(", fromlayer3 ");
            printf(" entity: %d\n", eventptr->eventity);
        }
        time = eventptr->evtime;        /* update time to next event time */

        if (nsim == nsimmax)    //전송이 끝날 때
            break;                        /* all done with simulation */

        if (eventptr->evtype == FROM_LAYER5) {  //새로운 패킷
            generate_next_arrival();   /* set up future arrival */
            /* fill in msg to give with string of same letter */
            j = nsim % 26;
            for (i = 0; i < 20; i++)
                msg2give.data[i] = 97 + j;
            if (TRACE > 2) {
                printf("          MAINLOOP: data given to student: ");
                for (i = 0; i < 20; i++)
                    printf("%c", msg2give.data[i]);
                printf("\n");
            }
            nsim++;
            if (eventptr->eventity == A)
                A_output(msg2give);
            else
                B_output(msg2give);
        }
        else if (eventptr->evtype == FROM_LAYER3) { //ACK
            pkt2give.seqnum = eventptr->pktptr->seqnum;
            pkt2give.acknum = eventptr->pktptr->acknum;
            pkt2give.checksum = eventptr->pktptr->checksum;
            for (i = 0; i < 20; i++)
                pkt2give.payload[i] = eventptr->pktptr->payload[i];
            if (eventptr->eventity == A)      /* deliver packet by calling */
                A_input(pkt2give);            /* appropriate entity */
            else
                B_input(pkt2give);
            free(eventptr->pktptr);          /* free the memory for packet */
        }
        else if (eventptr->evtype == TIMER_INTERRUPT) { //인터럽트 발생 시 함수 호출
            if (eventptr->eventity == A)
                A_timerinterrupt();
            else
                B_timerinterrupt();
        }
        else {
            printf("INTERNAL PANIC: unknown event type \n");
        }
        free(eventptr);
    }

terminate:
    printf(" Simulator terminated at time %f\n after sending %d msgs from layer5\n", time, nsim);
}



init()                         /* initialize the simulator */
{
    int i;
    float sum, avg;
    float jimsrand();


    printf("-----  Stop and Wait Network Simulator Version 1.1 -------- \n\n");
    printf("Enter the number of messages to simulate: ");
    scanf_s("%d", &nsimmax);
    printf("Enter  packet loss probability [enter 0.0 for no loss]:");
    scanf_s("%f", &lossprob);
    printf("Enter average time between messages from sender's layer5 [ > 0.0]:");
    scanf_s("%f", &lambda);
    printf("Enter Window size: ");
    scanf_s("%d", &windowSize);
    printf("Enter Timeout: ");
    scanf_s("%d", &RTT);

    printf("Enter TRACE:");
    scanf_s("%d", &TRACE);

    srand(9999);              /* init random number generator */
    sum = 0.0;                /* test random number generator for students */
    for (i = 0; i < 1000; i++)
        sum = sum + jimsrand();    /* jimsrand() should be uniform in [0,1] */
    avg = sum / 1000.0;
    if (avg < 0.25 || avg > 0.75) {
        printf("It is likely that random number generation on your machine\n");
        printf("is different from what this emulator expects.  Please take\n");
        printf("a look at the routine jimsrand() in the emulator code. Sorry. \n");
        exit();
    }

    ntolayer3 = 0;
    nlost = 0;
    ncorrupt = 0;

    time = 0.0;                    /* initialize time to 0.0 */
    generate_next_arrival();     /* initialize event list */
}

/****************************************************************************/
/* jimsrand(): return a float in range [0,1].  The routine below is used to */
/* isolate all random number generation in one location.  We assume that the*/
/* system-supplied rand() function return an int in therange [0,mmm]        */
/****************************************************************************/
float jimsrand()
{
    double mmm = 32767;   /* largest int  - MACHINE DEPENDENT!!!!!!!!   */
    float x;                   /* individual students may need to change mmm */
    x = rand() / mmm;            /* x should be uniform in [0,1] */
    return(x);
}

/********************* EVENT HANDLINE ROUTINES *******/
/*  The next set of routines handle the event list   */
/*****************************************************/

generate_next_arrival()
{
    double x, log(), ceil();
    struct event* evptr;
    char* malloc();
    float ttime;
    int tempint;

    if (TRACE > 2)
        printf("          GENERATE NEXT ARRIVAL: creating new arrival\n");

    x = lambda * jimsrand() * 2;  /* x is uniform on [0,2*lambda] */
                              /* having mean of lambda        */
    evptr = (struct event*)malloc(sizeof(struct event));
    evptr->evtime = time + x;
    evptr->evtype = FROM_LAYER5;
    //if (BIDIRECTIONAL && (jimsrand() > 0.5))
    //    evptr->eventity = B;
    //else
    //    evptr->eventity = A;
    evptr->eventity = A;    //메세지 sending은 항상 A로부터 이루어진다
    insertevent(evptr);
}


insertevent(p)
struct event* p;
{
    struct event* q, * qold;

    if (TRACE > 2) {
        printf("            INSERTEVENT: time is %lf\n", time);
        printf("            INSERTEVENT: future time will be %lf\n", p->evtime);
    }
    q = evlist;     /* q points to header of list in which p struct inserted */
    if (q == NULL) {   /* list is empty */
        evlist = p;
        p->next = NULL;
        p->prev = NULL;
    }
    else {
        for (qold = q; q != NULL && p->evtime > q->evtime; q = q->next)
            qold = q;
        if (q == NULL) {   /* end of list */
            qold->next = p;
            p->prev = qold;
            p->next = NULL;
        }
        else if (q == evlist) { /* front of list */
            p->next = evlist;
            p->prev = NULL;
            p->next->prev = p;
            evlist = p;
        }
        else {     /* middle of list */
            p->next = q;
            p->prev = q->prev;
            q->prev->next = p;
            q->prev = p;
        }
    }
}

void printevlist()
{
    struct event* q;
    int i;
    printf("--------------\nEvent List Follows:\n");
    for (q = evlist; q != NULL; q = q->next) {
        printf("Event time: %f, type: %d entity: %d\n", q->evtime, q->evtype, q->eventity);
    }
    printf("--------------\n");
}



/********************** Student-callable ROUTINES ***********************/

/* called by students routine to cancel a previously-started timer */
stoptimer(AorB)
int AorB;  /* A or B is trying to stop timer */
{
    struct event* q, * qold;

    if (TRACE > 2)
        printf("          STOP TIMER: stopping timer at %f\n", time);
    /* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
    for (q = evlist; q != NULL; q = q->next)
        if ((q->evtype == TIMER_INTERRUPT && q->eventity == AorB)) {
            /* remove this event */
            if (q->next == NULL && q->prev == NULL)
                evlist = NULL;         /* remove first and only event on list */
            else if (q->next == NULL) /* end of list - there is one in front */
                q->prev->next = NULL;
            else if (q == evlist) { /* front of list - there must be event after */
                q->next->prev = NULL;
                evlist = q->next;
            }
            else {     /* middle of list */
                q->next->prev = q->prev;
                q->prev->next = q->next;
            }
            free(q);
            return;
        }
    printf("Warning: unable to cancel your timer. It wasn't running.\n");
}


starttimer(AorB, increment)
int AorB;  /* A or B is trying to stop timer */
float increment;
{

    struct event* q;
    struct event* evptr;
    char* malloc();

    if (TRACE > 2)
        printf("          START TIMER: starting timer at %f\n", time);
    /* be nice: check to see if timer is already started, if so, then  warn */
   /* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
    for (q = evlist; q != NULL; q = q->next)
        if ((q->evtype == TIMER_INTERRUPT && q->eventity == AorB)) {
            printf("Warning: attempt to start a timer that is already started\n");
            return;
        }

    /* create future event for when timer goes off */
    evptr = (struct event*)malloc(sizeof(struct event));
    evptr->evtime = time + increment;
    evptr->evtype = TIMER_INTERRUPT;
    evptr->eventity = AorB;
    insertevent(evptr);
}


/************************** TOLAYER3 ***************/
tolayer3(AorB, packet)
int AorB;  /* A or B is trying to stop timer */
struct pkt packet;
{
    struct pkt* mypktptr;
    struct event* evptr, * q;
    char* malloc();
    float lastime, x, jimsrand();
    int i;


    ntolayer3++;

    /* simulate losses: */
    if (jimsrand() < lossprob) {
        nlost++;
        if (TRACE > 0)
            printf("          TOLAYER3: packet being lost\n");
        return;
    }

    /* make a copy of the packet student just gave me since he/she may decide */
    /* to do something with the packet after we return back to him/her */
    mypktptr = (struct pkt*)malloc(sizeof(struct pkt));
    mypktptr->seqnum = packet.seqnum;
    mypktptr->acknum = packet.acknum;
    mypktptr->checksum = packet.checksum;
    for (i = 0; i < 20; i++)
        mypktptr->payload[i] = packet.payload[i];
    if (TRACE > 2) {
        printf("          TOLAYER3: seq: %d, ack %d, check: %d ", mypktptr->seqnum,
            mypktptr->acknum, mypktptr->checksum);
        for (i = 0; i < 20; i++)
            printf("%c", mypktptr->payload[i]);
        printf("\n");
    }

    /* create future event for arrival of packet at the other side */
    evptr = (struct event*)malloc(sizeof(struct event));
    evptr->evtype = FROM_LAYER3;   /* packet will pop out from layer3 */
    evptr->eventity = (AorB + 1) % 2; /* event occurs at other entity */
    evptr->pktptr = mypktptr;       /* save ptr to my copy of packet */
  /* finally, compute the arrival time of packet at the other end.
     medium can not reorder, so make sure packet arrives between 1 and 10
     time units after the latest arrival time of packets
     currently in the medium on their way to the destination */
    lastime = time;
    /* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next) */
    for (q = evlist; q != NULL; q = q->next)
        if ((q->evtype == FROM_LAYER3 && q->eventity == evptr->eventity))
            lastime = q->evtime;
    evptr->evtime = lastime + 1 + 9 * jimsrand();


    //패킷이 손실될 경우는 고려하지 않으므로 배제한다.
    ///* simulate corruption: */
    //if (jimsrand() < corruptprob) {
    //    ncorrupt++;
    //    if ((x = jimsrand()) < .75)
    //        mypktptr->payload[0] = 'Z';   /* corrupt payload */
    //    else if (x < .875)
    //        mypktptr->seqnum = 999999;
    //    else
    //        mypktptr->acknum = 999999;
    //    if (TRACE > 0)
    //        printf("          TOLAYER3: packet being corrupted\n");
    //}

    if (TRACE > 2)
        printf("          TOLAYER3: scheduling arrival on other side\n");
    insertevent(evptr);
}

tolayer5(AorB, datasent)
int AorB;
char datasent[20];
{
    int i;
    if (TRACE > 2) {
        printf("          TOLAYER5: data received: ");
        for (i = 0; i < 20; i++)
            printf("%c", datasent[i]);
        printf("\n");
    }

}

float get_sim_time()
{
    return time;
}

int getwinsize()
{
    return windowSize;
}