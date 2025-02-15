#include <stdio.h>
#include <mpi.h>
#include <random>

#define rid_t int
#define COMM_SIZE 4

typedef struct {
    rid_t rank;

    rid_t aReceiver;
    rid_t bReceiver;
    rid_t aSender;
    rid_t bSender;

    int aGenerated;
    int bGenerated;
    int a;
    int b;
} MailInfo;

MailInfo getMailInfo(int rank) {
    MailInfo mi;
    mi.rank = rank;
    mi.a = -1;
    mi.b = -1;

    switch (rank) {
    case 0:
        mi.aReceiver = 2;
        mi.bReceiver = 1;
        mi.aSender = 1;
        mi.bSender = 2;
        break;
    case 1:
        mi.aReceiver = 0;
        mi.bReceiver = 3;
        mi.aSender = 3;
        mi.bSender = 0;
        break;
    case 2:
        mi.aReceiver = 3;
        mi.bReceiver = 0;
        mi.aSender = 0;
        mi.bSender = 3;
        break;
    case 3:
        mi.aReceiver = 1;
        mi.bReceiver = 2;
        mi.aSender = 2;
        mi.bSender = 1;
        break;
    }

    srand((unsigned int)(time(NULL) * (rank + 1)));
    mi.aGenerated = (rand() % 9) + 1;
    mi.bGenerated = (rand() % 9) + 1;

    return mi;
}

void serveRank(MailInfo * mi, int client) {
    if (client == mi->rank) {
        MPI_Send(&mi->aGenerated, 1, MPI_INT, mi->aReceiver, mi->rank * 10 + mi->aReceiver, MPI_COMM_WORLD);
        MPI_Send(&mi->bGenerated, 1, MPI_INT, mi->bReceiver, mi->rank * 10 + mi->bReceiver, MPI_COMM_WORLD);
        return;
    }

    if (client == mi->aSender) {
        MPI_Status s;
        MPI_Recv(&mi->a, 1, MPI_INT, mi->aSender, mi->aSender * 10 + mi->rank, MPI_COMM_WORLD, &s); 
        return;
    }

    if (client == mi->bSender) {
        MPI_Status s;
        MPI_Recv(&mi->b, 1, MPI_INT, mi->bSender, mi->bSender * 10 + mi->rank, MPI_COMM_WORLD, &s);
        return;
    }
}

int main(int argc, char ** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    if (size != COMM_SIZE) {
        if (rank == 0) printf("Expected Comm_size: %d. Got: %d\n", COMM_SIZE, size);
        MPI_Finalize();
        return EINVAL;
    }

    MailInfo mi = getMailInfo(rank);
    for (int i = 0; i < 4; i++) serveRank(&mi, i);
    printf("Process #%d Generated (%d, %d) Recieved (%d, %d) Result: %d\n", mi.rank, mi.aGenerated, mi.bGenerated, mi.a, mi.b, mi.a + mi.b);

    MPI_Finalize();
    return 0;
}
