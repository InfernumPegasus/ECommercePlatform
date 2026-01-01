#include <kafka/KafkaConsumer.h>
#include <kafka/KafkaProducer.h>

#include <csignal>
#include <iostream>
#include <nlohmann/json.hpp>
#include <pqxx/pqxx>
#include <string>
#include <thread>

#include "DbMapping.hpp"
#include "OrderService.hpp"
#include "User.hpp"
#include "UserProfile.hpp"

using namespace kafka;
using namespace kafka::clients;
using namespace kafka::clients::producer;
using namespace kafka::clients::consumer;

const std::string brokers = "localhost:9092";
const Topic topic = "test-topic";

void ProduceMessage() {
  // Create configuration object
  const Properties props({
      {"bootstrap.servers", {brokers}},
      {"enable.idempotence", {"true"}},
  });

  // Create a producer instance
  KafkaProducer producer(props);

  for (int i = 0; i < 10; i++) {
    const std::string line = "value_" + std::to_string(i);
    // The ProducerRecord doesn't own `line`, it is just a thin wrapper
    auto record = ProducerRecord(topic, NullKey, Value(line.c_str(), line.size()));
    // Send the message
    producer.send(
        record,
        // The delivery report handler
        [](const RecordMetadata& metadata, const Error& error) {
          if (!error) {
            std::cout << "% Message delivered: " << metadata.toString() << std::endl;
          } else {
            std::cerr << "% Message delivery failed: " << error.message() << std::endl;
          }
        },
        // The memory block given by record.value() would be copied
        KafkaProducer::SendOption::ToCopyRecordValue);
  }
}

void ConsumeMessage() {
  // Prepare the configuration
  const Properties props({{"bootstrap.servers", {brokers}}});

  // Create a consumer instance
  KafkaConsumer consumer(props);

  // Subscribe to topics
  consumer.subscribe({topic});

  for (int i = 0; i < 100; i++) {
    // Poll messages from Kafka brokers
    auto records = consumer.poll(std::chrono::milliseconds(100));

    for (const auto& record : records) {
      if (!record.error()) {
        std::cout << "Got a new message..." << std::endl;
        std::cout << "    Topic    : " << record.topic() << std::endl;
        std::cout << "    Partition: " << record.partition() << std::endl;
        std::cout << "    Offset   : " << record.offset() << std::endl;
        std::cout << "    Timestamp: " << record.timestamp().toString() << std::endl;
        std::cout << "    Headers  : " << toString(record.headers()) << std::endl;
        std::cout << "    Key   [" << record.key().toString() << "]" << std::endl;
        std::cout << "    Value [" << record.value().toString() << "]" << std::endl;
      } else {
        std::cerr << record.toString() << std::endl;
      }
    }
  }
}

std::string FormConnectionString(std::string_view dbname, std::string_view user,
                                 std::string_view password, std::string_view host,
                                 int port) {
  return std::format("dbname={} user={} password={} host={} port={}", dbname, user,
                     password, host, port);
}

std::string GetDbVersion(pqxx::work& work) {
  const pqxx::result res = work.exec("SELECT version()");
  return res.front().front().c_str();
}

struct OrderPrice {
  int64_t id;
  double total;
};

inline void to_json(nlohmann::json& j, const OrderPrice& p) {
  j = {{"id", p.id}, {"total", p.total}};
}

inline void from_json(const nlohmann::json& j, OrderPrice& p) {
  j.at("id").get_to(p.id);
  j.at("total").get_to(p.total);
}

template <>
struct DbMapping<OrderPrice> {
  static constexpr auto fields = std::make_tuple(std::pair{"id", &OrderPrice::id},
                                                 std::pair{"total", &OrderPrice::total});
};

template <>
struct DbMapping<User> {
  static constexpr auto fields =
      std::make_tuple(std::pair{"id", &User::id}, std::pair{"status", &User::status},
                      std::pair{"created_at", &User::created_at},
                      std::pair{"updated_at", &User::updated_at});
};

template <>
struct DbMapping<UserProfile> {
  static constexpr auto fields =
      std::make_tuple(std::pair{"user_id", &UserProfile::user_id},
                      std::pair{"display_name", &UserProfile::display_name});
};

void CalculateOrderPrice(pqxx::work& work) {
  static constexpr auto query =
      "SELECT id, calculate_order_total(id) AS total FROM \"Orders\";";

  const auto rows = work.exec(query).expect_columns(2);

  std::println(std::cout, "printing all prices:");

  for (auto r : rows) {
    const OrderPrice price = MapRowTo<OrderPrice>(r);

    std::println(std::cout, "{} = {}", price.id, price.total);
  }
}

void GetAllUsers(pqxx::work& work) {
  static constexpr auto query = "SELECT * FROM \"Users\";";

  const auto rows = work.exec(query).expect_columns(4);

  std::println(std::cout, "printing all users:");

  for (auto r : rows) {
    const User user = MapRowTo<User>(r);

    std::println(std::cout, "user {}: {}, created_at {}, updated_at {}", user.id,
                 user.status, user.created_at, user.updated_at);
  }
}

void GetAllUserProfiles(pqxx::work& work) {
  static constexpr auto query = "SELECT * FROM \"UserProfiles\";";

  const auto rows = work.exec(query).expect_columns(2);

  std::println(std::cout, "printing all user profiles:");

  for (auto r : rows) {
    const UserProfile user = MapRowTo<UserProfile>(r);

    std::println(std::cout, "user {}: {}", user.user_id,
                 user.display_name.value_or("NULL"));
  }
}

int main(int argc, char** argv) {
  try {
    const auto connectionString =
        FormConnectionString("ecommerce", "ecommerce", "ecommerce", "localhost", 5432);
    pqxx::connection conn(connectionString);
    pqxx::work work{conn};

    std::cout << "DB version: " << GetDbVersion(work) << std::endl;

    CalculateOrderPrice(work);
    GetAllUsers(work);
    GetAllUserProfiles(work);
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
