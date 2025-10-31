#include <taskflow/scheduler.hpp>
#include <gtest/gtest.h>
#include <atomic>
#include <vector>
#include <string>

using namespace std::chrono_literals;

class DAGTest : public ::testing::Test {
protected:
    void SetUp() override {
        scheduler = std::make_unique<tf::Scheduler>();
        scheduler->start();
    }
    
    void TearDown() override {
        if (scheduler) {
            scheduler->stop();
        }
    }
    
    std::unique_ptr<tf::Scheduler> scheduler;
};

TEST_F(DAGTest, BasicOrder) {
    std::atomic<int> order{0};

    auto t1 = scheduler->schedule_once(tf::Clock::now() + 50ms, [&]{ order = 1; });
    auto t2 = scheduler->schedule_once(tf::Clock::now() + 100ms, [&]{
        EXPECT_EQ(order, 1); 
        order = 2;
    }, {t1});

    scheduler->wait_for(t2);
    EXPECT_EQ(order, 2);
}

TEST_F(DAGTest, ChainedDependencies) {
    std::vector<int> execution_order;
    std::mutex order_mtx;

    auto add_to_order = [&](int id) {
        std::lock_guard<std::mutex> lk(order_mtx);
        execution_order.push_back(id);
    };

    auto t1 = scheduler->schedule_once(tf::Clock::now() + 10ms, [&]{ add_to_order(1); });
    auto t2 = scheduler->schedule_once(tf::Clock::now() + 10ms, [&]{ add_to_order(2); }, {t1});
    auto t3 = scheduler->schedule_once(tf::Clock::now() + 10ms, [&]{ add_to_order(3); }, {t2});

    scheduler->wait_for(t3);
    
    ASSERT_EQ(execution_order.size(), 3);
    EXPECT_EQ(execution_order[0], 1);
    EXPECT_EQ(execution_order[1], 2);
    EXPECT_EQ(execution_order[2], 3);
}

TEST_F(DAGTest, ParallelBranches) {
    std::vector<int> execution_order;
    std::mutex order_mtx;

    auto add_to_order = [&](int id) {
        std::lock_guard<std::mutex> lk(order_mtx);
        execution_order.push_back(id);
    };

    // Root task
    auto t1 = scheduler->schedule_once(tf::Clock::now() + 10ms, [&]{ add_to_order(1); });
    
    // Two parallel branches
    auto t2 = scheduler->schedule_once(tf::Clock::now() + 20ms, [&]{ add_to_order(2); }, {t1});
    auto t3 = scheduler->schedule_once(tf::Clock::now() + 20ms, [&]{ add_to_order(3); }, {t1});
    
    // Join task
    auto t4 = scheduler->schedule_once(tf::Clock::now() + 30ms, [&]{ add_to_order(4); }, {t2, t3});

    scheduler->wait_for(t4);
    
    ASSERT_EQ(execution_order.size(), 4);
    EXPECT_EQ(execution_order[0], 1);
    // t2 and t3 can execute in any order
    EXPECT_TRUE((execution_order[1] == 2 && execution_order[2] == 3) || 
                (execution_order[1] == 3 && execution_order[2] == 2));
    EXPECT_EQ(execution_order[3], 4);
}

TEST_F(DAGTest, RecurringTasks) {
    std::atomic<int> counter{0};
    
    auto recurring_task = scheduler->schedule_every(100ms, [&]{
        counter++;
    });
    
    std::this_thread::sleep_for(350ms); // Let it run ~3-4 times
    
    // Should run approximately every 100ms, so in 350ms expect ~3-4 runs
    // Allow some variance for CI timing but not exponential growth
    EXPECT_GE(counter.load(), 2); // Should have run at least 2 times
    EXPECT_LE(counter.load(), 6); // But not more than 6 times (with timing variance)
}