#include <iostream>
#include <map>
#include <mpi.h>
#include <unistd.h>
#include <math.h>

class Strategy {
public:
    ~Strategy() = default;

    Strategy() = default;

    virtual void execute() = 0;
};

class Context {
private:
    Strategy *strategy_;

public:
    Context(Strategy *strategy = nullptr) : strategy_(strategy) {

    }

    ~Context() {
        delete this->strategy_;
    }

    void setStrategy(Strategy *strategy) {
        delete this->strategy_;
        this->strategy_ = strategy;
    }

    void runStrategy() {
        this->strategy_->execute();
    }
};

class MPITask_1 : public Strategy {
public:
    void execute() override {
        MPI_Init(nullptr, nullptr);
        int rank, comm_size;
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
        printf("Hello world from %d/%d\n", rank, comm_size);
        MPI_Finalize();
    }
};

class MPITask_2 : public Strategy {
public:
    void execute() override {
        int rank, comm_size;
        int max = 0;
        int maxLocal = 0;

        int n = 100000;

        MPI_Init(nullptr, nullptr);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

        int local_size = n / comm_size;
        int *a = new int[n];
        int *receivebuf = new int[local_size];
        int *sendcounts = new int[comm_size];
        int *displs = new int[comm_size];
        if (rank == 0) {
            printf("Local size = %d\n", local_size);
            srand(time(NULL));//random seed
            for (int i = 0; i < n; i++) {
                a[i] = rand();
            }
            for (int i = 0; i < comm_size; ++i) {
                displs[i] = i * local_size;
                sendcounts[i] = local_size;
            }
        }
        MPI_Scatterv(a, sendcounts, displs, MPI_INT, receivebuf, local_size, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);
        for (int i = 0; i < local_size; i++) {
            if (receivebuf[i] > maxLocal) maxLocal = receivebuf[i];
        }
        printf("Local max = %d from process %d\n", maxLocal, rank);
        MPI_Reduce(&maxLocal, &max, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);
        if (rank == 0) {
            printf("Max = %d\n", max);
        }
        MPI_Finalize();
    }
};

class MPITask_3 : public Strategy {
public:
    void execute() override {
        int rank, comm_size;
        int count = 10000000;
        int condition_count = 0;
        double x, y;
        MPI_Init(NULL, NULL);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
        srand(rank*comm_size);
        int local_size = count / comm_size;
        int local_count = 0;

        for (int i = rank * local_size; i < rank * local_size + local_size; i++) {
            x = (double) rand() / (double) RAND_MAX;
            y = (double) rand() / (double) RAND_MAX;
            if (pow(x, 2) + pow(y, 2) <= 1)
                local_count++;
        }

        printf("Local count = %d from process %d\n", local_count, rank);
        MPI_Reduce(&local_count, &condition_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

        if (rank == 0) {
            double answer = (double) (4 * condition_count) / (double) count;
            printf("Pi = %f\n", answer);
        }
        MPI_Finalize();
    }
};

class MPITask_4 : public Strategy {
public:
    void execute() override {
        const int n = 10000;
        int sum = 0;

        int rank, comm_size;

        MPI_Init(NULL, NULL);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

        int local_size = n / comm_size;
        int *sendcounts = new int[comm_size];
        int *displs = new int[comm_size];
        int *arr = new int[n];
        int *receivebuff = new int[local_size];
        int sumLocal = 0;

        if (rank == 0) {
            printf("Local size = %d\n", local_size);
            srand(time(NULL));
            for (int i = 0; i < n; ++i) {
                arr[i] = rand();
            }
            for (int i = 0; i < comm_size; ++i) {
                sendcounts[i] = local_size;
                displs[i] = i * local_size;
            }
        }
        MPI_Scatterv(arr, sendcounts, displs, MPI_INT, receivebuff, local_size, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);

        for (int i = 0; i < local_size; ++i) {
            int number = receivebuff[i];
            if (number > 0) {
                sumLocal += number;
            }
        }

        printf("Local sum of procces #%d = %d\n", rank, sumLocal);

        MPI_Reduce(&sumLocal, &sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

        if (rank == 0) {
            printf("General sum = %d\n", sum);
            double result = sum * 1.0 / n;
            printf("Average of positive numbers = %.4f\n", result);
        }

        MPI_Finalize();
    }
};

class MPITask_5 : public Strategy {
public:
    void execute() override {
        int rank, comm_size;
        int n = 10000;

        MPI_Init(NULL, NULL);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

        int local_size = n / comm_size;
        int *a = new int[n];
        int *b = new int[n];
        long sumLocal = 0;
        long sum = 0;

        int *aLocal = new int[local_size];
        int *bLocal = new int[local_size];
        int *sendcounts = new int[comm_size];
        int *displs = new int[comm_size];

        if (rank == 0) {
            printf("Local size = %d\n", local_size);
            srand(time(NULL));
            for (int i = 0; i < n; ++i) {
                a[i] = rand() % 10;
                b[i] = rand() % 10;
            }
            for (int i = 0; i < comm_size; ++i) {
                sendcounts[i] = local_size;
                displs[i] = i * local_size;
            }
        }

        MPI_Scatterv(a, sendcounts, displs, MPI_INT, aLocal, local_size, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Scatterv(b, sendcounts, displs, MPI_INT, bLocal, local_size, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);
        for (int i = 0; i < local_size; ++i) {
            sumLocal += (long) (aLocal[i] * bLocal[i]);
        }
        printf("Local inner product #%d = %ld\n", rank, sumLocal);
        MPI_Reduce(&sumLocal, &sum, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
        if (rank == 0) {
            printf("Inner product = %ld\n", sum);
        }
        MPI_Finalize();
    }
};

class MPITask_6 : public Strategy {
public:
    void execute() override {
        const int n = 8;

        int matrix[n][n];

        int maxmin = INT32_MIN, minmax = INT32_MAX;
        int local_maxmin = maxmin;
        int local_minmax = minmax;

        int rank, comm_size;

        MPI_Init(NULL, NULL);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

        int local_size = n / comm_size;
        int *localMinimums = new int[local_size];
        int *localMaximums = new int[local_size];
        int part_to_process[local_size][n];

        if (rank == 0) {
            printf("Local size = %d\n", local_size);
            srand(time(NULL));
            for (int i = 0; i < n; ++i) {
                printf("| ");
                for (int j = 0; j < n; ++j) {
                    matrix[i][j] = rand() % 10;
                    printf("%d ", matrix[i][j]);
                }
                printf("|\n");
            }
        }
        MPI_Scatter(&matrix[0][0], local_size * n, MPI_INT, &part_to_process[0][0], local_size * n, MPI_INT, 0,
                    MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);

        for (int i = 0; i < local_size; ++i) {
            int localMin = INT32_MAX;
            int localMax = INT32_MIN;
            for (int j = 0; j < n; ++j) {
                if (localMin > part_to_process[i][j]) localMin = part_to_process[i][j];
                if (localMax < part_to_process[i][j]) localMax = part_to_process[i][j];
            }
            localMinimums[i] = localMin;
            localMaximums[i] = localMax;
        }
        for (int i = 0; i < local_size; ++i) {
            if (local_maxmin < localMinimums[i]) local_maxmin = localMinimums[i];
            if (local_minmax > localMaximums[i]) local_minmax = localMaximums[i];
        }

        printf("Local maxmin = %d, local minmax = %d\n", local_maxmin, local_minmax);

        MPI_Reduce(&local_maxmin, &maxmin, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);
        MPI_Reduce(&local_minmax, &minmax, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);

        if (rank == 0) {
            printf("maxmin = %d, minmax = %d\n", maxmin, minmax);
        }
        MPI_Finalize();
    }
};

class MPITask_7 : public Strategy {
public:
    void execute() override {
        int n = 4;
        int* a = new int[n*n];
        int x[n], y[n];
        int rank, comm_size;

        MPI_Init(NULL, NULL);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

        int local_size = n / comm_size;
        MPI_Datatype column_type;
        MPI_Type_vector(n, 1, n, MPI_INT, &column_type);
        MPI_Type_commit(&column_type);
        int* local_columns = new int[local_size*n];
        int* y_local = new int[n];
        int* x_local = new int[local_size];
        int* y_all = new int[n*comm_size];

        if (rank == 0) {
            printf("Local size = %d\n", local_size);
            srand(time(NULL));
            for (int i = 0; i < n; ++i) {
                x[i] = rand() % 2;
                for (int j = 0; j < n; ++j) {
                    a[i*n + j] = rand() % 10;
                }
            }
            printf("--INIT--\n");
            for (int i = 0; i < n; ++i) {
                printf("x[%d]=%d\n", i, x[i]);
            }
            for (int i = 0; i < n; ++i) {
                printf("| ");
                for (int j = 0; j < n; ++j) {
                    printf("%d ", a[i*n + j]);
                }
                printf("|\n");
            }
            printf("--    --\n");
        }
        MPI_Barrier(MPI_COMM_WORLD);
//        printf("Scatter x\n");
        MPI_Scatter(&x[0], local_size, MPI_INT, &x_local[0], local_size, MPI_INT, 0, MPI_COMM_WORLD);
//        printf("Scatter local columns\n");
//        MPI_Scatter(&a[0], local_size, column_type, &local_columns[0], local_size, column_type, 0,
//                    MPI_COMM_WORLD);
        MPI_Bcast(&a[0], n*n, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);

//        printf("Start calculating\n");
        for (int i = 0; i < n; ++i) {
            y_local[i] = 0;
            for (int j = 0; j < local_size; ++j) {
                y_local[i] = y_local[i] + a[i*n + j + rank*local_size] * x_local[j];
//                y_local[i] = y_local[i] + local_columns[i*n+j]* x_local[j];
            }
            printf("y_local[%d] = %d, process #%d\n", i, y_local[i], rank);
        }

        MPI_Gather(&y_local[0], n, MPI_INT, &y_all[0], n, MPI_INT, 0, MPI_COMM_WORLD);
        if (rank == 0) {
            for (int i = 0; i < n; ++i) {
                y[i] = 0;
                for (int j = 0; j < comm_size; ++j) {
                    y[i] = y[i] + y_all[j*n+i];
                }
                printf("y[%d] = %d\n", i, y[i]);
            }
        }
        MPI_Finalize();
    }
};

class MPITask_8 : public Strategy {
public:

    void execute() override {
        int n = 40;
        int rank, comm_size;

        MPI_Init(NULL, NULL);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

        int local_size = n/comm_size;
        int* a = new int[n];
        int* a_local = new int[local_size];
        int* new_a = new int[n];

        if (rank == 0){
            printf("Local size = %d\n", local_size);
            srand(time(NULL));
            printf("Array a: ");
            for (int i = 0; i < n; ++i) {
                a[i] = rand() % 10;
                printf("%d ", a[i]);
            }
            printf("\n");
            for (int i = 0; i < comm_size; ++i) {
                int* send = new int[local_size];
                for (int j = 0; j < local_size; ++j) {
                    send[j] = a[i*local_size + j];
                }
                if (i == 0){
                    a_local = send;
                } else {
                    MPI_Send(send, local_size, MPI_INT, i, i, MPI_COMM_WORLD);
                }
            }
        }
        if (rank != 0){
            MPI_Recv(a_local, local_size, MPI_INT, 0, rank, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
        }
        printf("Local #%d array: ", rank);
        for (int i = 0; i < local_size; ++i) {
            printf("%d ", a_local[i]);
        }
        printf("\n");

        if (rank != 0){
            MPI_Send(a_local, local_size, MPI_INT, 0, rank, MPI_COMM_WORLD);
        }
        MPI_Barrier(MPI_COMM_WORLD);
        if (rank == 0){
            for (int i = 0; i < comm_size; ++i) {
                if (i != 0){
                    MPI_Recv(a_local, local_size, MPI_INT, i, i, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
                }
                for (int j = 0; j < local_size; ++j) {
                    new_a[i*local_size + j] = a_local[j];
                }
            }
            printf("Array new_a: ");
            for (int i = 0; i < n; ++i) {
                printf("%d ", new_a[i]);
            }
        }
    }
};

class MPITask_9 : public Strategy {
public:
    void execute() override {
        int n = 40;
        int rank, comm_size;

        MPI_Init(NULL, NULL);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

        int local_size = n/comm_size;
        int* a = new int[n];
        int* a_reversed = new int[n];
        int* sendcounts = new int[comm_size];
        int* displs = new int[comm_size];
        int* reverse_displs = new int[comm_size];
        if (rank == 0) {
            printf("Local size = %d\n", local_size);
            srand(time(NULL));
            printf("ARRAY a: ");
            for (int i = 0; i < n; ++i) {
                a[i] = rand() % 10;
                printf("%d ", a[i]);
            }
            printf("\n");
        }
        MPI_Barrier(MPI_COMM_WORLD);
        sendcounts[0] = local_size;
        displs[0] = 0;
        reverse_displs[0] = n - local_size;

        for (int i = 1; i < comm_size; i++) {
            if (i == comm_size-1){
                sendcounts[i] = n - local_size*i;
            } else{
                sendcounts[i] = local_size;
            }
            displs[i] = displs[i - 1] + sendcounts[i - 1];
            reverse_displs[i] = reverse_displs[i - 1] - sendcounts[i];
        }
        int length = sendcounts[rank];
        printf("rank%d -> len = %d\n", rank, length);
        int* a_local = new int[length];

        MPI_Scatterv(a, sendcounts, displs, MPI_INT, a_local, length, MPI_INT, 0, MPI_COMM_WORLD);

        int* revers = new int[length];
        for(int i = 0; i < length; i++) {
            revers[i] = a_local[length - i - 1];
        }

        MPI_Gatherv(revers, length, MPI_INT, a_reversed, sendcounts, reverse_displs, MPI_INT, 0, MPI_COMM_WORLD);

        if(rank == 0) {
            printf("ARRAY a_reversed: ");
            for(int i = 0; i < n; i++)
                printf("%d ", a_reversed[i]);
            printf("\n");
        }

        MPI_Finalize();
    }
};

class MPITask_10 : public Strategy {
public:
    void execute() override {
        int n = 100000000;
        int rank, comm_size;
        MPI_Init(NULL, NULL);

        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

        int* a = new int[n];
        double start, end;
        if(rank == 0) {
            srand(time(NULL));
            for (int i = 0; i < n; i++) {
                a[i] = rand();
            }

            start = MPI_Wtime();
            MPI_Send(a, n, MPI_INT, 1, 0, MPI_COMM_WORLD);
            MPI_Recv(a, n, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
            end = MPI_Wtime();
            printf("Send = %f\n", end-start);

            start = MPI_Wtime();
            MPI_Ssend(a, n, MPI_INT, 1, 1, MPI_COMM_WORLD);
            MPI_Recv(a, n, MPI_INT, 1, 1, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
            end = MPI_Wtime();
            printf("Ssend = %f\n", end-start);

            int buffer_attached_size = MPI_BSEND_OVERHEAD + sizeof(int)*n;
            int* buffer_attached = (int*)malloc(buffer_attached_size);
            MPI_Buffer_attach(buffer_attached, buffer_attached_size);

            start = MPI_Wtime();
            MPI_Bsend(a, n, MPI_INT, 1, 2, MPI_COMM_WORLD);
            MPI_Recv(a, n, MPI_INT, 1, 2, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
            end = MPI_Wtime();
            printf("Bsend = %f\n", end-start);

            MPI_Buffer_detach(&buffer_attached, &buffer_attached_size);

            start = MPI_Wtime();
            MPI_Rsend(a, n, MPI_INT, 1, 3, MPI_COMM_WORLD);
            MPI_Recv(a, n, MPI_INT, 1, 3, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
            end = MPI_Wtime();
            printf("Rsend = %f\n", end-start);

        } else {
            MPI_Recv(a, n, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
            MPI_Send(a, n, MPI_INT, 0, 0, MPI_COMM_WORLD);

            MPI_Recv(a, n, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
            MPI_Ssend(a, n, MPI_INT, 0, 1, MPI_COMM_WORLD);

            int buffer_attached_size = MPI_BSEND_OVERHEAD + sizeof(int)*n;
            int* buffer_attached = (int*)malloc(buffer_attached_size);
            MPI_Buffer_attach(buffer_attached, buffer_attached_size);

            MPI_Recv(a, n, MPI_INT, 0, 2, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
            MPI_Bsend(a, n, MPI_INT, 0, 2, MPI_COMM_WORLD);

            MPI_Buffer_detach(&buffer_attached, &buffer_attached_size);

            MPI_Recv(a, n, MPI_INT, 0, 3, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
            MPI_Rsend(a, n, MPI_INT, 0, 3, MPI_COMM_WORLD);
        }

        MPI_Finalize();
    }
};

class MPITask_11 : public Strategy {
public:
    void execute() override {
        int rank, comm_size;
        MPI_Init(NULL, NULL);

        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

        int* send = new int[1];
        int* receive = new int[1];
        if (rank == 0) {
            send[0] = 0;
            MPI_Send(send, 1, MPI_INT, rank + 1, rank + 1, MPI_COMM_WORLD);
        }
        if (rank != 0) {
            MPI_Recv(receive, 1, MPI_INT, rank - 1, rank, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
            send[0] = receive[0] + 5;
            if (rank != comm_size - 1)
                MPI_Send(send, 1, MPI_INT, rank + 1, rank + 1, MPI_COMM_WORLD);
            else
                MPI_Send(send, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        }
        if (rank == 0) {
            MPI_Recv(receive, 1, MPI_INT, comm_size - 1, 0, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
            printf("result = %d\n", receive[0]);
        }

        MPI_Finalize();
    }
};

std::map<int, Strategy *> &getMap(std::map<int, Strategy *> &taskMapping);

int main() {
    std::map<int, Strategy *> taskMapping;
    taskMapping = getMap(taskMapping);

    int task = 3;

    if (task < 1 || task > taskMapping.size()) {
        return 0;
    }

    Context *context = new Context();
    context->setStrategy(taskMapping[task]);
    context->runStrategy();

    //end
    return 0;
}

std::map<int, Strategy *> &getMap(std::map<int, Strategy *> &taskMapping) {
    taskMapping[1] = new MPITask_1();
    taskMapping[2] = new MPITask_2();
    taskMapping[3] = new MPITask_3();
    taskMapping[4] = new MPITask_4();
    taskMapping[5] = new MPITask_5();
    taskMapping[6] = new MPITask_6();
    taskMapping[7] = new MPITask_7();
    taskMapping[8] = new MPITask_8();
    taskMapping[9] = new MPITask_9();
    taskMapping[10] = new MPITask_10();
    taskMapping[11] = new MPITask_11();
    return taskMapping;
}
