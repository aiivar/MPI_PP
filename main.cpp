#include <iostream>
#include <map>
#include <mpi.h>
#include <unistd.h>

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

class MPITask_1 : public Strategy{
public:
    void execute() override {
        MPI_Init(nullptr, nullptr);
        printf("Hello world\n");
        MPI_Finalize();
    }
};

class MPITask_2 : public Strategy{
public:
    void execute() override {
        int rank, comm_size;
        int max = 0;
        int maxReduce = 0;

        int arrLength = 10000;

        int err = MPI_Init(NULL, NULL);
        err = MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        err = MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

        int partition = arrLength / comm_size;
        int* arr = new int[arrLength];
        int* receivebuf = new int[partition]; // В самом поcледнем задании показано как правильно объявлять такие массивы (malloc)
        int* sendcounts = new int[comm_size];
        int* displs = new int[comm_size];
        if (rank == 0)
        {
            printf("Partition = %d\n", partition);
            srand(time(NULL));//random seed
            for (int i = 0; i < arrLength; i++)
            {
                arr[i] = rand();
            }
            for (int i = 0; i < comm_size; ++i) {
                displs[i] = i * partition;
                sendcounts[i] = partition;
            }
            printf("Scattering...\n");
        }
        err = MPI_Scatterv(arr, sendcounts, displs, MPI_INT, receivebuf, partition, MPI_INT, 0, MPI_COMM_WORLD);
        err = MPI_Barrier(MPI_COMM_WORLD);
        for (int i = 0; i < partition; i++)
        {
            if (receivebuf[i] > maxReduce) maxReduce = receivebuf[i];
        }
        printf("Sending max %d to rank 0 from rank %d\n", maxReduce, rank);
        err = MPI_Reduce(&maxReduce, &max, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);
        if (rank == 0)
        {
            printf("Max = %d\n", max);
        }
        err = MPI_Finalize();
    }
};

std::map<int, Strategy *> &getMap(std::map<int, Strategy *> &taskMapping);

int main() {
    std::map<int, Strategy *> taskMapping;
    taskMapping = getMap(taskMapping);

    int task;

    printf("## Enter number of the task:");

    scanf("%d", &task);

    if (task < 1 || task > taskMapping.size()) {
        return 0;
    }

    Context *context = new Context();
    context->setStrategy(taskMapping[task]);
    context->runStrategy();
    printf("## End of task #%d", task);

    //end
    return 0;
}

std::map<int, Strategy *> &getMap(std::map<int, Strategy *> &taskMapping) {
    taskMapping[1] = new MPITask_1();
    taskMapping[2] = new MPITask_2();
    return taskMapping;
}
