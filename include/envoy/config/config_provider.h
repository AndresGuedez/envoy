#pragma once

#include <memory>

#include "envoy/common/time.h"

#include "common/protobuf/protobuf.h"

#include "absl/types/optional.h"

namespace Envoy {
namespace Config {

/**
 * A provider for configuration obtained ether statically or dynamically via xDS APIs.
 *
 * Use config() to obtain a shared_ptr to the implementation of the config, and configProtoInfo() to
 * obtain a reference to the underlying config proto and version (applicable only to dynamic config
 * providers).
 */
class ConfigProvider {
public:
  /**
   * The "implementation" of the configuration.
   * Use config() to obtain a typed object that corresponds to the specific configuration
   * represented by this abstract type.
   */
  class Config {
  public:
    virtual ~Config() {}
  };
  using ConfigConstSharedPtr = std::shared_ptr<const Config>;

  /**
   * Stores the config proto as well as the associated version.
   */
  template <typename P> struct ConfigProtoInfo {
    const P& config_proto_;

    // Only populated by dynamic config providers.
    std::string version_;
  };

  virtual ~ConfigProvider() {}

  /**
   * Returns a ConfigProtoInfo associated with the provider.
   * @return absl::optional<ConfigProtoInfo<P>> an optional ConfigProtoInfo; the value is set when a
   * config is available.
   */
  template <typename P> absl::optional<ConfigProtoInfo<P>> configProtoInfo() const {
    static_assert(std::is_base_of<Protobuf::Message, P>::value,
                  "Proto type must derive from Protobuf::Message");

    const auto* config_proto = dynamic_cast<const P*>(getConfigProto());
    if (config_proto == nullptr) {
      return {};
    }
    return ConfigProtoInfo<P>{*config_proto, getConfigVersion()};
  }

  /**
   * Returns the Config corresponding to the provider.
   * @return std::shared_ptr<const C> a shared pointer to the Config.
   */
  template <typename C> std::shared_ptr<const C> config() const {
    static_assert(std::is_base_of<Config, C>::value,
                  "Config type must derive from ConfigProvider::Config");

    return std::dynamic_pointer_cast<const C>(getConfig());
  }

  /**
   * Returns the timestamp associated with the last update to the Config.
   * @return SystemTime the timestamp corresponding to the last config update.
   */
  virtual SystemTime lastUpdated() const PURE;

protected:
  /**
   * Returns the config proto associated with the provider.
   * @return Protobuf::Message* the config proto corresponding to the Config instantiated by the
   *         provider.
   */
  virtual const Protobuf::Message* getConfigProto() const PURE;

  /**
   * Returns the config version associated with the provider.
   * @return std::string the config version.
   */
  virtual std::string getConfigVersion() const PURE;

  /**
   * Returns the config implementation associated with the provider.
   * @return ConfigConstSharedPtr the config as the base type.
   */
  virtual ConfigConstSharedPtr getConfig() const PURE;
};

using ConfigProviderPtr = std::unique_ptr<ConfigProvider>;

} // namespace Config
} // namespace Envoy
