#include <kafka/KafkaProducer.h>

#include <csignal>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>

#include "OrderService.hpp"

int main(int argc, char** argv) {
  using namespace kafka;
  using namespace kafka::clients;
  using namespace kafka::clients::producer;

  const std::string brokers = "localhost:9092";
  const Topic topic = "test-topic";

  try {
    // Create configuration object
    const Properties props({
        {"bootstrap.servers", {brokers}},
        {"enable.idempotence", {"true"}},
    });

    // Create a producer instance
    KafkaProducer producer(props);

    // Read messages from stdin and produce to the broker
    std::cout
        << "% Type message value and hit enter to produce message. (empty line to quit)"
        << std::endl;

    for (std::string line; std::getline(std::cin, line);) {
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

      if (line.empty()) break;
    }

    // producer.close(); // No explicit close is needed, RAII will take care of it

  } catch (const KafkaException& e) {
    std::cerr << "Unexpected exception caught: " << e.what() << std::endl;
  }
}
