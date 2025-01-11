#pragma once

#include "ots-exceptions.hpp"
#include <memory>
#include <string>
#include <vector>
#include <array>
#include <map>
#include <utility>

/**
 * @file ots.hpp
 * @brief Header for the C++ library
 * @see ots.h for the C ABI
 */

/**
 * @namespace ots
 * @brief The library exists complete only in this namespace
 *
 * This header file provides following features for the ots library for C++:
 *
 * - Legacy Seed (13 words) handling - except generating, omited on purpose
 * - Monero Seed (25 words) handling
 * - Polyseed (16 words) handling
 * - Address management
 * - Address verification
 * - Import of outputs
 * - Export of key images
 * - inspect unsigned transaction
 * - sign a unsigned transaction
 */
namespace ots {

    /**
     * @enum Network
     * @brief Represents the monero network
     *
     * - MAIN: production and default network
     * - TEST: development and testing network
     * - STAGE: pre-production testing
     */
	enum class Network {
		MAIN,
		TEST,
		STAGE,
        // WARNING: On extending the network types, the cryptonoteNetwork function in ots-internal.hpp needs to be updated, or at least reviewed.
        // WARNING: keep OTS_NETWORK in sync with this enum, too!
	};

    /**
     * @enum AddressType
     * @brief Represents the monero address type
     *
     * Currently a monero address can be one of the following:
     * - [Standard](https://docs.getmonero.org/public-address/standard-address/)
     * - [SubAddress](https://docs.getmonero.org/public-address/subaddress/)
     * - [Integrated Address](https://docs.getmonero.org/public-address/integrated-address/)
     */
    enum class AddressType {
        Standard,
        SubAddress,
        Integrated
        // WARNING: Keep OTS_ADDRESS_TYPE in sync with this enum!
    };

    /**
     * @enum SeedType
     * @brief Represents the seed type, and is used for the different dictionaries (languages) available
     *
     * - Monero: Languages for Monero Seeds (@see LegacySeed, @see MoneroSeed)
     * - Polyseed: Languages for @see Polyseed
     */
	enum class SeedType {
		Monero,
		Polyseed
        // WARNING: Keep OTS_SEED_TYPE in sync with this enum!
	};

	// Forward declarations
	class Seed;
    class Address;
	class Wallet;
    class KeyStore;
    class WipeableString;
    class SeedIndices;
    struct KeyStoreDeleter { // for std::unique_ptr, if not it will not compile
            void operator()(KeyStore* p) const;
    };
    /**
     * @class Account
     * @brief holds monero internal account and internal functions
     * @internal
     */
    class Account;
    struct AccountDeleter { // for std::unique_ptr, if not it will not compile
            void operator()(Account* p) const;
    };
    class PolyseedKeyStore;
	class TxDescription;
	class TxWarning;

	// Type aliases
	using key_handle_t = size_t;
	using seed_handle_t = size_t;

    /**
     * @class SeedLanguage
     * @brief Manages seed phrase languages and their properties
     * 
     * Provides methods to retrieve, validate, and manage seed phrase languages
     */
	class SeedLanguage {
		public:
            /**
             * @brief Retrieves the native name of the seed language
             * @return std::string Language native name
             */
			[[nodiscard]] const std::string& name() const;

            /**
             * @brief Retrieves the English name of the seed language
             * @return std::string Language name in English
             */
			[[nodiscard]] const std::string& englishName() const;

            /**
             * @brief Retrieves the two-letter language code, which could be extended by a dash with a variation
             * @return std::string language code
             */
			[[nodiscard]] const std::string& code() const;

            /**
             * @brief Checks if the language is supported for a specific seed type
             * @param type SeedType to check support for (default: Monero), alternative Polyseed
             * @return bool True if the language is supported, false otherwise
             */
			[[nodiscard]] bool supported(SeedType type = SeedType::Monero) const;

            /**
             * @brief Checks if this is the default language for a given seed type
             * @param type SeedType to check (default: Monero), alternative Polyseed
             * @return True if this is the default language, false otherwise
             */
			[[nodiscard]] bool isDefault(SeedType type = SeedType::Monero) const;

            /**
             * @brief The native language index
             * @param type SeedType to check support for (default: Monero), alternative Polyseed
             * @return index for the SeedType or -1 on unkown
             */
			[[nodiscard]] int index(SeedType type = SeedType::Monero) const noexcept;

            /**
             * @brief Compares two SeedLanguage objects for equality
             * @param other SeedLanguage to compare against
             * @return bool True if languages are the same, false otherwise
             */
			bool operator==(const SeedLanguage& other) const;

            /**
             * @brief Compares two SeedLanguage objects for equality
             * @param other SeedLanguage to compare against
             * @return bool True if languages are the same, false otherwise
             */
			bool operator==(SeedLanguage& other) const;

            /**
             * @brief Compares SeedLanguage to a language code
             * @param code language code to compare against
             * @return bool True if language codes are the same, false otherwise
             */
			bool operator==(const std::string& code) const;

            /**
             * @brief returns the english name as string
             * @return std::string
             */
            operator const std::string&() const;

            /**
             * @brief Retrieves a SeedLanguage by its native name
             * @param name Native language name
             * @return const SeedLanguage Found language object
             * @throws std::runtime_error If language not found
             */
			static const SeedLanguage& fromName(const std::string& name);

            /**
             * @brief Retrieves a SeedLanguage by its English name
             * @param name English language name
             * @return const SeedLanguage Found language object
             * @throws std::runtime_error if language not found
             */
			static const SeedLanguage& fromEnglishName(const std::string& name);

            /**
             * @brief Retrieves a SeedLanguage by its language code
             * @param code Language code
             * @return const SeedLanguage Found language object
             * @throws std::runtime_error If language not found
             */
			static const SeedLanguage& fromCode(const std::string& code);


            /**
             * @brief Retrieves the complete list of available seed languages
             * @return const std::vector<SeedLanguage>& List of all supported languages
             */
			static const std::vector<std::reference_wrapper<const SeedLanguage>> list();

            /**
             * @brief Retrieves languages supported for a specific seed type
             * @param type SeedType to filter languages: (default: Monero), alternative Polyseed
             * @return const std::vector<SeedLanguage> Languages supporting the seed type
             */
			static const std::vector<std::reference_wrapper<const SeedLanguage>> listFor(SeedType type);

            /**
             * @brief Gets the default language for a given seed type
             * @param type SeedType to find default language (default: Monero), alternative Polyseed
             * @return const SeedLanguage& Default language for the seed type
             */
			static const SeedLanguage& defaultLanguage(SeedType type = SeedType::Monero);

            /**
             * @brief Sets the default language for a given seed type
             * @param type SeedType to set default language for
             * @param language SeedLanguage to set as default
             * @throws ots::exception::language::LanguageNotFound If the language is not supported
             */
            static void setDefaultLanguage(SeedType type, const SeedLanguage& language);

		private:
			static std::vector<SeedLanguage> s_list;
			static std::map<SeedType, std::reference_wrapper<const SeedLanguage>> s_default;
			std::string m_code;
			std::string m_name;
			std::string m_englishName;
			std::map<SeedType, bool> m_supported;
			std::map<SeedType, int> m_index;
	};

    /**
     * @class Seed
     * @brief Abstract base class for cryptographic seed management
     * 
     * Provides core functionality for managing seeds
     */
	class Seed {
		public:
            /**
             * @brief Virtual destructor to ensure proper cleanup of derived classes
             */
			virtual ~Seed() = default;

            /**
             * @brief Generates the seed phrase in a specified language
             * @param language SeedLanguage to generate phrase in
             * @return Seed phrase
             * @throws ots::exception::seed::UnsupportedLanguage If the language is not supported
             */
			virtual const WipeableString phrase(const SeedLanguage& language) const = 0;

            /**
             * @brief Gets the raw numeric values representing the seed (indices)
             * @return Seed numeric representation
             */
			virtual const SeedIndices indices() const = 0;

            /**
             * @brief Provides a unique fingerprint for the seed
             * @return std::string Seed fingerprint
             *
             * @note Fingerprint is the last 6 digits of sha256(address) as uppercase hex
             * @note Can't thow exception, because on constructing a seed address must be valid or throw and exception. And with valid address the fingerprint is also valid.
             */
			virtual const std::string& fingerprint() const noexcept;

            /**
             * @brief Address of the seed
             * @return reference to the address of the seed
             *
             * @note Can't thow exception, because on constructing a seed address must be valid or throw and exception.
             */
			virtual const Address& address() const noexcept;

            /**
             * @brief Gets the seed's creation timestamp
             * @return uint64_t Seed creation timestamp
             *
             * For Polyseed this is more exact as height()
             * For Monero seeds it depends what was provided by the user,
             *  if there date was provided it will be more accurate, in
             *  case the block height on creation was provide this is also
             *  only a rough estimation
             */
			virtual const uint64_t timestamp() const noexcept;

            /**
             * @brief Gets the blockchain height associated with the seed
             * @return uint64_t Blockchain height
             *
             * For Polyseed this is an estimation minus a security time span of 30 days in the past
             * For Monero seeds it will be the same if the user provided a date, otherwise
             *  it will be the height the user provided, can be 0 also.
             */
			virtual const uint64_t height() const noexcept;

            /**
             * @brief Checks if the seed is encrypted
             * @return bool True if the seed is encrypted, false otherwise
             */
			virtual inline bool encrypted() const noexcept { return false; };

            /**
             * @brief Gets the network associated with the seed
             * @return Network Cryptocurrency network
             *
             * Main importance is that the addresses are based on the key
             * and the network - other network different keys
             */
			virtual inline const Network& network() const noexcept { return m_network; };

            /**
             * @brief Creates a wallet from the seed
             * @return std::shared_ptr<Wallet> Wallet created from the seed
             *
             * will return always the same wallet, wallet is created only once.
             */
            virtual std::shared_ptr<Wallet> wallet();
            // Explicitly delete copy operations
            Seed(const Seed&) = delete;
            Seed& operator=(const Seed&) = delete;
            // Allow move operations
            Seed(Seed&&) noexcept = default;
            Seed& operator=(Seed&&) noexcept = default;

            /**
             * @brief Merges two seed values, so you can join multiple SeedQRs to one seed
             * @param values1 First seed values to merge
             * @param values2 Second seed values to merge
             * @return std::vector<uint16_t> Merged seed values
             * @throws ots::exception::seed::LengthMismatch If the seeds are not of the same size
             *
             * @note The vectors need to have the same size
             */
            static std::vector<uint16_t> mergeValues(const std::vector<uint16_t>& values1, const std::vector<uint16_t>& values2);

            /**
             * @brief Merges multiple seed values, so you can join multiple SeedQRs to one seed
             * @param values the collection of seed values to merge
             * @return std::vector<uint16_t> Merged seed values
             * @throws ots::exception::seed::LengthMismatch If the seeds are not of the same size
             * @throws ots::exception::seed::TooFewValues if not at least two values are provided
             *
             * @note The vectors need to have the same size, and at least two values need to be provided
             */
            static std::vector<uint16_t> mergeValues(const std::vector<std::vector<uint16_t>>& values);

            /**
             * @brief Merges two seed values, so you can join multiple SeedQRs to one seed
             * @param values1 First seed values to merge
             * @param values2 Second seed values to merge
             * @param del will delete both values after merging and zeroize them, default is true
             * @return std::vector<uint16_t> Merged seed values
             * @throws ots::exception::seed::LengthMismatch If the seeds are not of the same size
             *
             * @note The vectors need to have the same size
             */
            static std::vector<uint16_t> mergeAndZeorizeValues(std::vector<uint16_t>& values1, std::vector<uint16_t>& values2, bool del = true);

            /**
             * @brief Merges multiple seed values, so you can join multiple SeedQRs to one seed
             * @param values the collection of seed values to merge
             * @param del will delete all values after merging and zeroize them, default is true
             * @return std::vector<uint16_t> Merged seed values
             * @throws ots::exception::seed::LengthMismatch If the seeds are not of the same size
             * @throws ots::exception::seed::TooFewValues if not at least two values are provided
             *
             * @note The vectors need to have the same size, and at least two values need to be provided
             */
            static std::vector<uint16_t> mergeAndZeorizeValues(std::vector<std::vector<uint16_t>>& values, bool del = true);

		protected:
            Seed();
            std::unique_ptr<Address> m_address;
            std::shared_ptr<Wallet> m_wallet;
			uint64_t m_timestamp = 0;
			uint64_t m_height = 0;
            std::unique_ptr<KeyStore, KeyStoreDeleter> m_key;
			Network m_network;
	};


    /**
     * @class EncryptableSeed
     * @brief Abstract base class for seeds that support encryption
     * 
     * Extends Seed with encryption and decryption capabilities
     */
	class EncryptableSeed : public Seed {
		public:
            /**
             * @brief Encrypts the seed with a password
             * @param password Encryption password
             * @return bool True if encrypted, false otherwise
             */
			virtual bool encrypt(const std::string& password) = 0;

            /**
             * @brief Decrypts the seed with a password
             * @param password Decryption password
             * @return bool True if decrypted, false otherwise
             */
			virtual bool decrypt(const std::string& password) = 0;
        protected:
            EncryptableSeed() = default;
	};


    /**
     * @class LegacySeed
     * @brief Represents a monero 13-words seed type for backward compatibility
     * 
     * Provides only decoding
     */
	class LegacySeed : public Seed {
		public:
            /**
             * @brief seed phrase in a specified language
             * @param language SeedLanguage to use
             * @return std::string Seed phrase
             */
			const WipeableString phrase(const SeedLanguage& language) const override;
			const SeedIndices indices() const override;

            /**
             * @brief Decodes a seed from a phrase
             * @param phrase Seed phrase
             * @param language SeedLanguage of the phrase
             * @param height Optional blockchain height
             * @param time Optional timestamp
             * @param network Network type (default: MAIN)
             * @return LegacySeed Decoded seed
             */
			static LegacySeed decode(
					const std::string& phrase,
					const SeedLanguage& language, 
					uint64_t height = 0,
					uint64_t time = 0, 
					Network network = Network::MAIN
					);

            /**
             * @brief Decodes a seed from numeric values
             * @param values Numeric representation of the seed
             * @param height Optional blockchain height
             * @param time Optional timestamp
             * @param network Network type (default: MAIN)
             * @return LegacySeed Decoded seed
             */
			static LegacySeed decode(
					const std::vector<uint16_t>& values, 
					uint64_t height = 0,
					uint64_t time = 0, 
					Network network = Network::MAIN
					);
	};

    /**
     * @class MoneroSeed
     * @brief Represents a Monero 25-words seed
     * 
     * Provides Monero seed generation and decoding
     */
	class MoneroSeed : public EncryptableSeed {
		public:
			const WipeableString phrase(const SeedLanguage& language) const override;
			const SeedIndices indices() const override;
			bool encrypted() const noexcept override;
			bool encrypt(const std::string& password) override;
			bool decrypt(const std::string& password) override;
            /**
             * @brief Decodes a Monero seed from a phrase
             * @param phrase Seed phrase
             * @param height Optional blockchain height
             * @param time Optional timestamp
             * @param network Network type (default: MAIN)
             * @param password Decryption password, if different from empty string
             * @return MoneroSeed Decoded seed
             *
             * @note autodetect Language
             * @note password in combination with monero seeds is unluckily a mess, I guess in monero source itself it is called seed offset because the password if used for the wallet file encryption. In cryptonois it is used for the seed encryption (seed offset). But I think outside from monero source password makes more sense.
             */
			static MoneroSeed decode(
					const std::string& phrase,
					uint64_t height = 0,
					uint64_t time = 0, 
					Network network = Network::MAIN,
                    const std::string& password = ""
					);

			static MoneroSeed decode(
					const std::vector<uint16_t>& values, 
					uint64_t height = 0,
					uint64_t time = 0, 
					Network network = Network::MAIN,
                    const std::string& password = ""
					);

            /**
             * @brief Creates a Monero seed from a random source
             * @param random 32-byte random input
             * @param height Optional blockchain height
             * @param time Optional timestamp
             * @param network Network type (default: MAIN)
             * @return MoneroSeed created seed
             *
             * Can be used to:
             * - recover a seed
             * - generate from external entropy
             */
			static MoneroSeed create(
                    const std::array<unsigned char, 32>& random,
					uint64_t height = 0,
					uint64_t time = 0, 
					Network network = Network::MAIN,
                    const SeedLanguage& language = SeedLanguage::defaultLanguage(SeedType::Monero)
					);

            /**
             * @brief Generates a new Monero seed
             * @param height Optional blockchain height
             * @param time Optional timestamp
             * @param network Network type (default: MAIN)
             * @return MoneroSeed Generated seed
             *
             * @warning Generates a seed from the device provided entropy.
             *          Use with caution if the device can't provide good
             *          enough entropy!
             */
			static MoneroSeed generate(
					uint64_t height = 0,
					uint64_t time = 0, 
					Network network = Network::MAIN,
                    const SeedLanguage& language = SeedLanguage::defaultLanguage(SeedType::Monero)
					);
        protected:
            MoneroSeed() = default;
            bool m_encrypted = false;
	};

    /**
     * @class Polyseed
     * @brief Represents a Polyseed type
     * 
     * Provides Polyseed generation, encoding, and management
     */
	class Polyseed : public EncryptableSeed {
		public:
			const WipeableString phrase(const SeedLanguage& language) const override;
			const SeedIndices indices() const override;
            MoneroSeed moneroSeed() const;
			bool encrypted() const noexcept override;

            /**
             * @throw ots::exception::keystore::polyseed::ActivePolyseedDataSession should never happen
             * @throw ots::exception::keystore::LockedWriteAttempt should never happen
             * @throw ots::exception::keystore::LockedAccessAttempt should never happen
             */
			bool encrypt(const std::string& password) override;

            /**
             * @throw ots::exception::keystore::polyseed::ActivePolyseedDataSession should never happen
             * @throw ots::exception::keystore::LockedWriteAttempt should never happen
             * @throw ots::exception::keystore::LockedAccessAttempt should never happen
             */
			bool decrypt(const std::string& password) override;

            /**
             * @brief Creates a Polyseed with specific parameters
             * @param time Timestamp for seed creation, default 0, what translates to now (current system time)
             * @param network Network type (default: MAIN)
             * @return Polyseed Created seed
             */
			static Polyseed create(
					uint64_t time = 0,
					Network network = Network::MAIN
					);

            /**
             * @brief Decodes a Polyseed from a phrase
             * @param phrase Seed phrase
             * @param network Network type (default: MAIN)
             * @return Polyseed Decoded seed
             *
             * @note autodetect language
             */
			static Polyseed decode(
					const std::string& phrase, 
					Network network = Network::MAIN
					);

            /**
             * @brief Decodes a Polyseed from a phrase
             * @param phrase Seed phrase
             * @param language the language of the wordlist to use
             * @param network Network type (default: MAIN)
             * @return Polyseed Decoded seed
             */
			static Polyseed decode(
					const std::string& phrase, 
					const SeedLanguage& language, 
					Network network = Network::MAIN
					);

            /**
             * @brief Decodes a Polyseed from indexes of the words
             * @param values Seed phrase as index representation
             * @param network Network type (default: MAIN)
             * @return Polyseed Decoded seed
             *
             * @note Language not needed because the index is the same in all languages
             */
			static Polyseed decode(
					const std::vector<uint16_t>& values, 
					Network network = Network::MAIN
					);
        protected:
            Polyseed() = default;
            std::unique_ptr<PolyseedKeyStore> m_seed;
	};

    /**
     * @class Address
     * @brief Represents a Monero address
     *
     * Ensures valid Monero address and some insight of the address
     */
	class Address {
		public:
            /**
             * @brief create and check and monero address
             * @param address the base58 encoded address
             * @throws ots::addressInvalid if not a valid monero address
             */
			explicit Address(const std::string& address);

            /**
             * @brief Get the network of the address
             * @return the network type of the address
             */
			[[nodiscard]] const Network network() const noexcept;

            /**
             * @brief Get the type of the address
             * @return the type of the address
             */
			[[nodiscard]] const AddressType type() const noexcept;

            /**
             * @brief Generates a unique fingerprint for the Address
             * @return std::string Address fingerprint
             * @note Fingerprint is the last 6 digits of sha256(address) as upper case hex
             */
			virtual const std::string& fingerprint() const noexcept;

            /**
             * @brief Is it an integrated address
             * An integrated address is a standard address with a payment ID
             * @return bool True if the address is integrated, false otherwise
             */
            bool isIntegrated() const noexcept;

            /**
             * @brief Get the payment ID of the address
             * @return std::string Payment ID or empty string if not integrated
             */
            std::string paymentID() const noexcept;

            /**
             * @brief Get the standard address for an integrated address
             * @return Address standard address
             * @throws ots::exception::address::NotIntegrated if the address is not integrated
             */
            Address integratedAddress() const;

            /**
             * @brief character length of the base58 address
             * @return size_t length of the address
             */
			[[nodiscard]] const size_t length() const noexcept;

            /**
             * @brief let you use the Address like it would be a std::string
             * @return base58 address as std::string
             */
			explicit operator std::string() const noexcept;

            /**
             * @brief let you use the Address like it would be a const std::string&
             * @return base58 address as std::string
             */
			operator const std::string&() const noexcept;

            /**
             * @brief let you use the Address like it would be a uit8_t*
             * @return base58 address as uint8_t*
             */
			operator const uint8_t*() const noexcept;

            /**
             * @brief Compare Address with a std::string address
             * @return bool True if the addresses are the same, false otherwise
             */
            bool operator==(const std::string& other) const noexcept;

            /**
             * @brief check if a given string is a valid monero address
             * @param address the base58 encoded address
             * @throws ots::exception::address::Invalid if not a valid monero address
             *
             * @return the network the address should be valid for
             */
			[[nodiscard]] static Network network(const std::string& address);

            /**
             * @brief Get the type of the address
             * @param address the base58 encoded address
             * @return the type of the address
             * @throws ots::exception::address::Invalid if not a valid monero address
             */
			[[nodiscard]] static AddressType type(const std::string& address);

            /**
             * @brief check if a given string is a valid monero address
             * @param address the base58 encoded address
             * @param network the network the address should be valid for
             * @return base58 address as std::string
             */
			[[nodiscard]] static bool isValid(const std::string& address, const Network network = Network::MAIN) noexcept;

            /**
             * @brief check if a given string is a valid monero address, and and integrated address
             * @param address the base58 encoded address
             * @return true if an it is integrated address
             * @throws ots::exception::address::Invalid if not a valid monero address
             */
            [[nodiscard]] static bool isIntegrated(const std::string& address);

            /**
             * @brief check if a given string is a valid monero address, and and integrated address
             * @param address the base58 encoded address
             * @param network the network the address should be valid for, default is MAIN
             * @return the payment ID of the address, if it is an integrated address, else an empty string
             * @throws ots::exception::address::Invalid if not a valid monero address
             */
            [[nodiscard]] static std::string paymentID(const std::string& address, Network network = Network::MAIN);

            /**
             * @brief check if a given string is a valid monero address, and and integrated address
             * @param address the base58 encoded address
             * @param network the network the address should be valid for, default is MAIN
             * @return the base58 encoded integrated address
             * @throws ots::exception::address::Invalid if not a valid monero address
             * @throws ots::exception::address::NotIntegrated if the address is not an integrated address
             */
            [[nodiscard]] static std::string integratedAddress(const std::string& address, Network network = Network::MAIN);

		protected:
            explicit Address(const std::string& address, Network network);
            static Address validAddress(const std::string& address);
			std::string m_address;
            Network m_network;
            AddressType m_type;
			mutable std::string m_fingerprint;
	};

    /**
     * @class OTS
     * @brief General functionality
     */
	class OTS {
		public:
			explicit OTS();

            /**
             * @return the version liker major.minor.patch
             */
			static const std::string version() noexcept;

            /**
             * @return {major, minor, patch}
             */
			static std::array<int, 3> versionComponents() noexcept;

            /**
             * @brief Estimates the block height of a given timestamp
             * @param timestamp unix timestamp (epoch, seconds since 1970-01-01 00:00:00 UTC)
             * @param network estimation is network related
             * @return monero blockchain block height
             */
			static uint64_t heightFromTimestamp(uint64_t timestamp, Network network = Network::MAIN);

            /**
             * @brief Estimates the timestamp for a given block height on a network
             * @param height monero blockchain block height
             * @param network estimation is network related
             * @return timestamp unix timestamp (epoch, seconds since 1970-01-01 00:00:00 UTC)
             */
			static uint64_t timestampFromHeight(uint64_t height, Network network = Network::MAIN);

            /**
             * @brief Random 32 bytes
             * @return 32 random bytes
             * @warning Entropy is depending on your device, on low entropy devices don't use this for security related purpose!
             */
            static std::array<unsigned char, 32> random();

            /**
             * @brief Random bytes
             * @param[IN] size size of bytes
             * @param[OUT] bytes where to write the random bytes
             * @warning Entropy is depending on your device, on low entropy devices don't use this for security related purpose!
             */
            static void random(size_t size, uint8_t *bytes);
	};

    /**
     * @class SeedJar
     * @brief Storage for the Seed objects, mainly for the purpose of the C ABI
     * @todo check if not better include in OTS
     */
	class SeedJar {
		public:
            /**
             * @brief Stores a Seed object and returns a handle, to retrieve later
             * @param seed the actual Seed object to store
             * @return the handle to retrieve the Seed object with @see get(seed_handle_t)
             *
             * @note Attempt to store various times the same Seed object, results in receiving same handle,
             *       without duplicating reference of the Seed object
             */
			seed_handle_t store(const Seed& seed) const noexcept;
            
            /**
             * @brief retrieve a Seed object via a handle
             * @param handle received on @see store(Seed)
             * @return The Seed object if in the jar
             * @throws ots::exception::seedjar::SeedNotFound if the Seed object is not in the jar
             */
			const Seed& get(seed_handle_t handle) const;

            /**
             * @brief retrieve a Seed object from the jar from the fingerprint of the Seed object
             * @param fingerprint 6digit fingerprint of the Seed object
             * @return The Seed object if in the jar
             * @throws ots::exception::seedjar::SeedNotFound if the Seed object is not in the jar
             */
			const Seed& get(const std::string& fingerprint) const;
            uint32_t count() const noexcept;
			bool has(seed_handle_t handle) const noexcept;
			bool has(const std::string& fingerprint) const;
			std::vector<std::reference_wrapper<const Seed>> list() const noexcept;
			static const SeedJar& instance() noexcept;
	};

    /**
     * @class Wallet
     * @brief provides all offline wallet functionality
     */
	class Wallet {
		public:

            /**
             * @brief The block height on creation of the wallet
             * @return (estimated or user provided) restore block height, worst case
             *         is 0, from the beginning of the block chain
             * @note For the library itself meaningless, because of it's offline
             *       character, but is important for information to the view only
             *       wallet.
             */
            uint64_t height() const noexcept { return m_height; };

            /**
             * @brief Generate/lookup address in wallet
             * @param account 0 is the default account and wallet itself
             * @param index starting from 0 is the index of the subaddress in the account
             * @return Standard address for account 0 and index 0 else the subaddress
             */
			Address address(uint32_t account = 0, uint32_t index = 0) const noexcept;

            /**
             * @brief Gives a list of accounts (e.g. for a address browser)
             * @param max maximum entries returned, which should be always be return, except if index + max < max(uint32_t)
             * @param offset first account to return
             * @return the list of the addresses which represent the account (subaddress index 0)
             * @see https://docs.getmonero.org/public-address/subaddress
             */
			std::vector<Address> accounts(uint32_t max = 10, uint32_t offset = 0) const noexcept;

            /**
             * @brief Gives a list of subaddresses for a given account
             * @param account index of the account, 0 is the wallet itself
             * @param max maximum entries returned, which should be always be return, except if index + max < max(uint32_t)
             * @param offset first subindex to return
             * @return the list of the addresses which represent subaddresses of the account (subaddress index 0 is the account itself)
             */
			std::vector<Address> subAddresses(uint32_t account = 0, uint32_t max = 10, uint32_t offset = 0) const noexcept;

            /**
             * @brief check if the address string is a valid monero address and then if the address belongs to the wallet
             * @param address the string containing supposingly a monero address
             * @return false if the address is not valid or not belong to the wallet, otherwise true
             */
			bool hasAddress(const std::string& address) const noexcept;

            /**
             * @brief check if the address belongs to the wallet
             * @param address the monero address
             * @return true if the address belongs to the wallet
             */
			bool hasAddress(const Address& address) const noexcept;

            /**
             * @brief check if the address string is a valid monero address and give the account and subindex
             * @param address the string containing supposingly a monero address
             * @throws ots::exception::address::Invalid if the address is not valid
             * @throws ots::exception::wallet::AddressNotFound if the address is not in the wallet
             * @return first number is the account, second number the index
             */
			std::pair<uint32_t, uint32_t> addressIndex(const std::string& address) const;

            /**
             * @brief get the account and subindex of a address in the wallet
             * @param address the string containing supposingly a monero address
             * @throws ots::exception::address::Invalid if the address is not valid
             * @throws ots::exception::wallet::AddressNotFound if the address is not in the wallet
             * @return first number is the account, second number the index
             */
			std::pair<uint32_t, uint32_t> addressIndex(const Address& address) const;

            /**
             * @brief The Secret View Key
             * @return the wallet secret view key
             */
            WipeableString secretViewKey() const noexcept;

            /**
             * @brief The Public View Key
             * @return the wallet public view key
             */
            WipeableString publicViewKey() const noexcept;

            /**
             * @brief The Secret Spend Key
             * @return the wallet secret spend key
             */
            WipeableString secretSpendKey() const noexcept;

            /**
             * @brief The Public Spend Key
             * @return the wallet public spend key
             */
            WipeableString publicSpendKey() const noexcept;

            /**
             * @brief import the output, previous exported from the view only wallet
             * @return count of imported outputs
             * @throws ots::exception::wallet::ImportOutputs if provides outputs are not valid
             * @note the outputs are needed to export the key images @see Wallet::importOutputs()
             */
			uint64_t importOutputs(const std::string& outputs);

            /**
             * @brief export key images after outputs are imported
             * @return key images for the provided outputs
             * @throws ots::exception::wallet::ExportKeyImages if there are no key images, probably because no outputs are imported
             * @note the key images are needed by the view only wallet to create an unsigned transaction
             */
			WipeableString exportKeyImages() const;

            /**
             * @brief Gives a whole picture of the unsigned transaction
             * @param unsignedTransaction unsigned transaction string
             * @throws ots::exception::tx::Invalid if the provided transaction is invalid
             * @return a Transaction Description
             */
			TxDescription describeTransaction(const std::string& unsignedTransaction) const;

            /**
             * @brief Gives a warnings if any for the unsigned transaction
             * @param unsignedTransaction unsigned transaction string
             * @throws ots::exception::tx::Invalid if the provided transaction is invalid
             * @return a list of warnings
             */
			std::vector<TxWarning> checkTransaction(const std::string& unsignedTransaction) const;

            /**
             * @brief Gives a warnings if any for the unsigned transaction
             * @param description Transaction Description object
             * @return a list of warnings
             */
			std::vector<TxWarning> checkTransaction(const TxDescription& description) const noexcept;

            /**
             * @brief Sign a provided unsigned transaction
             * @param unsignedTransaction unsigned transaction string
             * @throws ots::exception::tx::Invalid if the provided transaction is invalid
             * @return a signed transaction
             */
			std::string signTransaction(const std::string& unsignedTransaction) const;

            /**
             * @brief Sign a provided string
             * @param data String of the message to sign
             * @return signature of the message
             */
			std::string signData(const std::string& data) const noexcept;

            /**
             * @brief Verify a signed message
             * @param data String of the message to sign
             * @param address Monero address as string
             * @param signature the provided signature for the message
             * @throws ots::exception::address::Invalid if the address is not valid
             * @return true if the signature is valid
             */
			bool verifyData(
					const std::string& data, 
					const std::string& address, 
					const std::string& signature
					) const;

            /**
             * @brief Create a offline Wallet from a secret key
             * @param key provided as insecure std::array, will be stored in the keystore, make sure to wipe the provided variable after creating the Wallet
             * @param height restore height of the wallet
             * @param network the network of the wallet
             */
            Wallet(const std::array<unsigned char, 32>& key, uint64_t height, const Network network) noexcept;

            /**
             * @brief Create a offline Wallet from a secret key
             * @param key provide the key via a secret key Storage
             * @param height restore height of the wallet
             * @param network the network of the wallet
             * @internal Only for internal use how KeyStore is not publicly declared
             */
            Wallet(const KeyStore& key, uint64_t height, const Network network) noexcept;
        protected:
            std::unique_ptr<KeyStore, KeyStoreDeleter> m_key;
            std::unique_ptr<Account, AccountDeleter> m_account;
            Network m_network;
            uint64_t m_height = 0;
	};

    /**
     * @class TxDescription
     * @brief detailed information about a transaction, used to check transaction before signing
     * @todo Figure out how to best structure - must be done during actual implementation
     */
	class TxDescription {};

    /**
     * @class TxWarning
     * @brief Warnings directed to the actual user related to a transaction to be signed,
     *        to make it easier to the application developer to help the user make informed
     *        decissions.
     * @todo Figure out how to best offer this kind of warnings (i18n?)
     */
	class TxWarning {};

    // set this on the end because there will be a lot of bloat in the documentation.
    /**
     * @brief String class that wipes its memory on destruction
     * @note This class helps prevent sensitive data from remaining in memory,
     *       but does not guarantee complete memory security
     */
    class WipeableString {

        public:
            /** @brief Default constructor */
            WipeableString() noexcept = default;

            /** @brief Construct from std::string */
            inline explicit WipeableString(const std::string& s): m_str(s) {};

            /** @brief Construct from C-string */
            explicit WipeableString(const char* s);

            /** @brief Construct from string and length */
            WipeableString(const char* s, size_t n);

            /** @brief Copy constructor */
            WipeableString(const WipeableString& other);
            
            /** @brief Move constructor */
            WipeableString(WipeableString&& other) noexcept;
            
            /** @brief Copy assignment operator */
            WipeableString& operator=(const WipeableString& other);
            
            /** @brief Move assignment operator */
            WipeableString& operator=(WipeableString&& other) noexcept;

            /** @brief Assignment operator for C-string */
            WipeableString& operator=(const char* s);

            /** @brief Destructor that wipes memory */
            ~WipeableString();

            /**
             * @brief Create an insecure std::string copy
             * @warning The returned string will not be wiped from
             *          memory automatically. Use only if you are sure
             *          what you are and why you are doing.
             */
            std::string insecure() const noexcept;

            /** @brief Get C-string representation */
            const char* c_str() const noexcept;

            /** @brief Get string data */
            const char* data() const noexcept;

            /** @brief Get string length */
            size_t size() const noexcept;

            /** @brief Check if string is empty */
            bool empty() const noexcept;

            /** @brief Clear string content */
            void clear() noexcept;

            /** @brief Get string capacity */
            size_t capacity() const noexcept;

            /** @brief Reserve memory for string */
            void reserve(size_t n);

            /** @brief Get substring */
            WipeableString substr(size_t pos = 0, size_t len = std::string::npos) const;

            /** @brief Find substring */
            size_t find(const WipeableString& str, size_t pos = 0) const noexcept;
            
            /** @brief Find C-string */
            size_t find(const char* s, size_t pos = 0) const noexcept;

            /** @brief Append string */
            WipeableString& append(const WipeableString& str);

            /** @brief Append C-string */
            WipeableString& append(const char* s);

            /** @brief Append operator */
            WipeableString& operator+=(const WipeableString& str);

            /** @brief Append C-string operator */
            WipeableString& operator+=(const char* s);

            /** @brief Compare with another WipeableString */
            bool operator==(const WipeableString& other) const noexcept;

            /** @brief Compare inequality with another WipeableString */
            bool operator!=(const WipeableString& other) const noexcept;

            /**
             * @brief Conversion to std::string is explicitly prevented
             * @throws ots::exception::wipeablestring::UnsafeConversion
             * @warning This is done to prevent accidental insecure conversions
             *          of sensitive data. Use insecure() method to get
             *          an insecure copy of the string if you really need it.
             */
            operator std::string() const;

            /** @brief Compare strings */
            int compare(const WipeableString& other) const noexcept;

            /** @brief Stream output operator */
            friend std::ostream& operator<<(std::ostream& os, const WipeableString& str);

            /** @brief Static npos value */
            static constexpr size_t npos = std::string::npos;

        private:
            /** @brief Internal string storage */
            std::string m_str;

            /** @brief Wipe the string's memory */
            void wipe() noexcept;
    };

    /**
     * @brief Secure container for seed word indices
     * @note Memory is automatically wiped on destruction
     */
    class SeedIndices {
        public:
            /** @brief Default constructor */
            SeedIndices() noexcept = default;

            /** @brief Construct from std::vector */
            inline explicit SeedIndices(const std::vector<uint16_t>& v): m_vec(v) {};

            /** @brief Construct with size */
            explicit SeedIndices(size_t size): m_vec(size) {};

            /** @brief Construct with size and value */
            SeedIndices(size_t size, uint16_t value): m_vec(size, value) {};

            /** @brief Copy constructor */
            SeedIndices(const SeedIndices& other): m_vec(other.m_vec) {};

            /** @brief Move constructor */
            SeedIndices(SeedIndices&& other) noexcept: m_vec(std::move(other.m_vec)) {};

            /** @brief Copy assignment operator */
            SeedIndices& operator=(const SeedIndices& other);

            /** @brief Move assignment operator */
            SeedIndices& operator=(SeedIndices&& other) noexcept;

            /** @brief Destructor that wipes memory */
            ~SeedIndices();

            /** @brief Get vector size */
            size_t size() const noexcept;

            /** @brief Check if vector is empty */
            bool empty() const noexcept;

            /** @brief Clear indices */
            void clear() noexcept;

            /** @brief Element access */
            uint16_t& operator[](size_t pos);
            const uint16_t& operator[](size_t pos) const;
            uint16_t& at(size_t pos);
            const uint16_t& at(size_t pos) const;

            /** @brief Add index */
            void push_back(uint16_t value);

        private:
            /** @brief Internal storage */
            std::vector<uint16_t> m_vec;

            /** @brief Wipe the memory */
            void wipe() noexcept;
    };
} // namespace ots
