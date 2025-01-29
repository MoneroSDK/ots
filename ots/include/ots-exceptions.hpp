#pragma once
#include <vector>

/**
 * @file ots-exceptions.hpp
 * @brief Header for the C++ library Exceptions
 */

#ifdef __PRETTY_FUNCTION__
#define NOT_IMPLEMENTED_YET() throw ots::exception::NotImplementedYet(__PRETTY_FUNCTION__)
#else
#define NOT_IMPLEMENTED_YET() throw ots::exception::NotImplementedYet(__func__)
#endif

#define REGISTER_EXCEPTION(CLASS, CODE, NAME) \
    static constexpr int32_t error_code = CODE; \
    static constexpr const char* error_class = NAME; \
    protected: \
    static inline bool registered = ots::exception::Exception::registerException<CLASS>(error_code, error_class);


#include <stdexcept>
#include <string>

namespace ots {
    /**
     * @namespace ots::exception
     * @brief All ots library related exceptions are in this namespace
     * @see https://en.cppreference.com/w/cpp/header/stdexcept
     *
     * @note in the REGISTER_EXCEPTION statement in the classes,
     *       the C error codes are defined which is the second parameter
     *       
     */
    namespace exception {
        /**
         * @class Exception
         * @internal
         * @brief Base for all OTS exceptions with virtual what()
         */
        class Exception {
            public:
                struct RegisteredException {
                    int32_t error_code;
                    const char* error_class;
                    const std::type_info& type;
                };

                static std::vector<RegisteredException>& registry() {
                    static std::vector<RegisteredException> instances;
                    return instances;
                }

            protected:
                template<typename T>
                    static bool registerException(int32_t code, const char* cls) {
                        registry().push_back({code, cls, typeid(T)});
                        return true;
                    }

            public:
                virtual ~Exception() = default;
                virtual const char* what() const noexcept = 0;
        };

        /**
         * @class RuntimeError
         * @brief Conditions only detectable at runtime
         * @internal
         * @note Should not used directly, is to override only
         */
        class RuntimeError: public std::runtime_error, public Exception {
            REGISTER_EXCEPTION(RuntimeError, -1, "RuntimeError")
            public:
                inline explicit RuntimeError(const std::string& msg): std::runtime_error(msg) {};
                const char* what() const noexcept override { return std::runtime_error::what(); }
        };

        /**
         * @class LogicError
         * @brief violation of logical preconditions or class invariants
         * @internal
         * @note Should not used directly, is to override only
         */
        class LogicError: public std::logic_error, public Exception {
            REGISTER_EXCEPTION(LogicError, -2, "LogicError")
            public:
                inline explicit LogicError(const std::string& msg): std::logic_error(msg) {};
                const char* what() const noexcept override { return std::logic_error::what(); }
        };

        /**
         * @class InvalidArgument
         * @brief provided an invalid argument
         * @internal
         * @note Should not used directly, is to override only
         */
        class InvalidArgument: public std::invalid_argument, public Exception {
            REGISTER_EXCEPTION(InvalidArgument, -3, "InvalidArgument")
            public:
                inline explicit InvalidArgument(const std::string& msg): std::invalid_argument(msg) {};
                const char* what() const noexcept override { return std::invalid_argument::what(); }
        };

        /**
         * @class RangeError
         * @brief range error in internal computation
         * @note Should not used directly, is to override only
         */
        class RangeError: public std::range_error, public Exception {
            REGISTER_EXCEPTION(RangeError, -4, "RangeError")
            public:
                inline explicit RangeError(const std::string& msg): std::range_error(msg) {};
                const char* what() const noexcept override { return std::range_error::what(); }
        };

        /**
         * @class NotImplementedYet
         * @brief Sole purpose of this exception is to create already the stub and mark as incomplete, you should never get such exception in the complete library
         */
        class NotImplementedYet: public std::logic_error, public Exception {
            REGISTER_EXCEPTION(NotImplementedYet, -5, "NotImplementedYet")
            public:
                inline NotImplementedYet(): std::logic_error("Not implemented yet") {};
                inline explicit NotImplementedYet(const std::string& msg): std::logic_error(std::string("Not implemented yet: ") + msg) {};
                const char* what() const noexcept override { return std::logic_error::what(); }
        };

        /**
         * @class DomainError
         * @brief errors related to the library use itself
         * @note Should not used directly, is to override only
         */
        class DomainError: public std::domain_error, public Exception {
            REGISTER_EXCEPTION(DomainError, -6, "DomainError")
            public:
                inline explicit DomainError(const std::string& msg): std::domain_error(msg) {};
                const char* what() const noexcept override { return std::domain_error::what(); }
        };

        /**
         * @class OutOfRange
         * @brief arguments outside of expected range. Needle not in Haystack ;)
         * @internal
         * @note Should not used directly, is to override only
         */
        class OutOfRange: public std::out_of_range, public Exception {
            REGISTER_EXCEPTION(OutOfRange, -7, "OutOfRange")
            public:
                inline explicit OutOfRange(const std::string& msg): std::out_of_range(msg) {};
                const char* what() const noexcept override { return std::out_of_range::what(); }
        };

        /**
         * @class BufferOverflowException
         * @brief size is bigger then buffer
         */
        class BufferOverflowException: OutOfRange {
            REGISTER_EXCEPTION(BufferOverflowException, -71, "BufferOverflowException")
            public:
                inline explicit BufferOverflowException(): OutOfRange("The size is bigger as the buffer! Security related exception!") {};
                inline explicit BufferOverflowException(const std::string& msg): OutOfRange(msg) {};
        };

        /**
         * @class MemoryAllocationError
         * @brief Could not allocate memory
         */
        class MemoryAllocationError: RuntimeError {
            REGISTER_EXCEPTION(MemoryAllocationError, -11, "MemoryAllocationError")
            public:
                inline explicit MemoryAllocationError(): RuntimeError("Couldn't allocate memory!") {};
        };

        /**
         * @class SecurityException
         * @brief Any security realted issue
         */
        class SecurityException: DomainError {
            REGISTER_EXCEPTION(SecurityException, -61, "SecurityException")
            public:
                inline explicit SecurityException(const std::string& msg): DomainError(std::string("SECURITY EXCEPTION: ") + msg) {};
        };

        /**
         * @class SecurityError
         * @brief Any security realted error: like no random generator available
         */
        class SecurityError: RuntimeError {
            REGISTER_EXCEPTION(SecurityError, -12, "SecurityError")
            public:
                inline explicit SecurityError(const std::string& msg): RuntimeError(std::string("SECURITY ERROR: ") + msg) {};
        };

        /**
         * @class LowEntropy
         * @brief Low entropy detected
         */
        class LowEntropy: SecurityError {
            REGISTER_EXCEPTION(LowEntropy, -121, "LowEntropy")
            public:
                inline explicit LowEntropy(): SecurityError("Low entropy detected!") {};
        };

        /**
         * @class UnknownNetwork
         * @brief a network is not known
         */
        class UnknownNetwork: DomainError {
            REGISTER_EXCEPTION(UnknownNetwork, -62, "UnknownNetwork")
            public:
                inline explicit UnknownNetwork(): DomainError("Unknown network") {};
        };

        /**
         * @namespace ots::exception::wipeablestring
         * @brief All exceptions related to ots::WipeableString
         * @note error codes start with -10xx
         */
        namespace wipeablestring {

            /**
             * @class UnsafeConversion
             * @brief a conversion to std::string is not allowed
             */
            class UnsafeConversion: SecurityException {
                REGISTER_EXCEPTION(UnsafeConversion, -1001, "UnsafeConversion")
                public:
                    inline explicit UnsafeConversion(): SecurityException("Unsafe conversion to std::string, use wipeableString.insecure() instead, if you really need to do this!") {};
            };
        }

        /**
         * @namespace ots::exception::wipeablevector
         * @brief All exceptions related to ots::WipeableVector
         * @note error codes start with -20xx
         */
        namespace wipeablevector {
            /**
             * @brief a conversion to std::vector is not allowed
             */
            class UnsafeConversion: SecurityException {
                REGISTER_EXCEPTION(UnsafeConversion, -2001, "UnsafeConversion")
                public:
                    inline explicit UnsafeConversion(): SecurityException("Unsafe conversion to std::vector, use wipeableVector.insecure() instead, if you really need to do this!") {};
            };
        }

        /**
         * @namespace ots::exception::sign
         * @brief All exceptions related to sign data
         * @note error codes start with -30xx
         */
        namespace sign {
            /**
             * @class InvalidSignature
             * @brief a signature is invalid
             */
            class InvalidSignature: DomainError {
                REGISTER_EXCEPTION(InvalidSignature, -3001, "InvalidSignature")
                public:
                    inline explicit InvalidSignature(): DomainError("Invalid signature") {};
                    inline explicit InvalidSignature(const std::string& msg): DomainError(msg) {};
            };

            /**
             * @class EmptyMessage
             * @brief a message is empty
             */
            class EmptyMessage: DomainError {
                REGISTER_EXCEPTION(EmptyMessage, -3002, "EmptyMessage")
                public:
                    inline explicit EmptyMessage(): DomainError("Empty message") {};
            };

            /**
             * @class EmptySignature
             * @brief a signature is empty
             */
            class EmptySignature: DomainError {
                REGISTER_EXCEPTION(EmptySignature, -3003, "EmptySignature")
                public:
                    inline explicit EmptySignature(): DomainError("Empty signature") {};
            };
        }

        /**
         * @namespace ots::exception::seed
         * @brief All exceptions related to ots::Seed
         * @note error codes start with -40xx
         */
        namespace seed {
            /**
             * @class NoDefaultLanguageSet
             * @brief a default language is expected but not set
             */
            class NoDefaultLanguageSet: DomainError {
                REGISTER_EXCEPTION(NoDefaultLanguageSet, -4001, "NoDefaultLanguageSet")
                public:
                    inline explicit NoDefaultLanguageSet(): DomainError("No default language set for seed type") {};
            };

            /**
             * @class LanguageNotFound
             * @brief a language expected if not found
             */
            class LanguageNotFound: OutOfRange {
                REGISTER_EXCEPTION(LanguageNotFound, -4002, "LanguageNotFound")
                public:
                    inline explicit LanguageNotFound(): OutOfRange("Language not found") {};
                    inline explicit LanguageNotFound(const std::string& msg): OutOfRange(msg) {};
            };

            /**
             * @class LanguageNotSupportedBySeedType
             * @brief a language is not supported by the seed type
             */
            class LanguageNotSupportedBySeedType: DomainError {
                REGISTER_EXCEPTION(LanguageNotSupportedBySeedType, -4003, "LanguageNotSupportedBySeedType")
                public:
                    inline explicit LanguageNotSupportedBySeedType(): DomainError("Language not supported by seed type") {};
            };

            /**
             * @class WordCount
             * @brief a seed expected has a different word count. Polyseed are 16 words, Monero Seeds are 25 words, Legacy Seeds are 13 words
             */
            class WordCount: OutOfRange {
                REGISTER_EXCEPTION(WordCount, -4004, "WordCount")
                public:
                    inline explicit WordCount(): OutOfRange("Wrong number of words in seed phrase") {};
            };

            /**
             * @class SeedEncodingFailed
             * @brief a seed could not be encoded
             */
            class SeedEncodingFailed: DomainError {
                REGISTER_EXCEPTION(SeedEncodingFailed, -4005, "SeedEncodingFailed")
                public:
                    inline explicit SeedEncodingFailed(): DomainError("Unable to encode seed") {};
                    inline explicit SeedEncodingFailed(const std::string& msg): DomainError(msg) {};
            };

            /**
             * @class SeedDecodingFailed
             * @brief a seed could not be encoded
             */
            class SeedDecodingFailed: DomainError {
                REGISTER_EXCEPTION(SeedDecodingFailed, -4006, "SeedDecodingFailed")
                public:
                    inline explicit SeedDecodingFailed(): DomainError("Unable to decode seed") {};
                    inline explicit SeedDecodingFailed(const std::string& msg): DomainError(msg) {};
            };

            /**
             * @class LengthMismatch
             * @brief a seed expected has a different length, and is used instead of WordCount on merging Values/seed phrases together.
             *        The length of the seed phrases or values must be the same, you can't merge a 25 words Monero Seed with a 16 words Polyseed e.g.
             */
            class LengthMismatch: DomainError {
                REGISTER_EXCEPTION(LengthMismatch, -4007, "LengthMismatch")
                public:
                    inline explicit LengthMismatch(): DomainError("Seed length mismatch") {};
            };

            /**
             * @class TooFewValues
             * @brief you need at least two values to merge.
             */
            class TooFewValues: DomainError {
                REGISTER_EXCEPTION(TooFewValues, -4008, "TooFewValues")
                public:
                    inline explicit TooFewValues(): DomainError("Too few values provided") {};
            };

            /**
             * @class PasswordNotSupported
             * @brief a password is not supported for the seed type
             */
            class PasswordNotSupported: DomainError {
                REGISTER_EXCEPTION(PasswordNotSupported, -4009, "PasswordNotSupported")
                public:
                    inline explicit PasswordNotSupported(): DomainError("Password not supported for seed type") {};
            };

            /**
             * @class MergeError
             * @brief a seed index could not be merged
             */
            class MergeError : public DomainError {
                REGISTER_EXCEPTION(MergeError, -4010, "MergeError")
                public:
                    explicit MergeError(const std::string& msg) : DomainError(msg) {}
            };
        }

        /**
         * @namespace ots::exception::polyseed
         * @brief All exceptions related to ots::Polyseed
         * @note error codes start with -50xx
         */
        namespace polyseed {
            /**
             * @class WordCount
             * @brief a polyseed expected has a different word count. Polyseed are 16 words
             */
            class WordCount: ots::exception::seed::WordCount {
                REGISTER_EXCEPTION(WordCount, -5001, "WordCount")
            };

            /**
             * @class UnsupportedLanguage
             * @brief a language is not supported by polyseed
             */
            class UnsupportedLanguage: ots::exception::seed::LanguageNotFound {
                REGISTER_EXCEPTION(UnsupportedLanguage, -5002, "UnsupportedLanguage")
                public:
                    inline explicit UnsupportedLanguage(): ots::exception::seed::LanguageNotFound("Unsupported Language") {};
            };

            /**
             * @class ChecksumMismatch
             * @brief a checksum mismatch detected
             */
            class ChecksumMismatch: DomainError {
                REGISTER_EXCEPTION(ChecksumMismatch, -5003, "ChecksumMismatch")
                public:
                    inline explicit ChecksumMismatch(): DomainError("Checksum mismatch") {};
            };

            /**
             * @class UnsupportedFeatures
             * @brief a feature is not supported by polyseed
             */
            class UnsupportedFeatures: DomainError {
                REGISTER_EXCEPTION(UnsupportedFeatures, -5004, "UnsupportedFeatures")
                public:
                    inline explicit UnsupportedFeatures(): DomainError("One or more features are not supported") {};
            };

            /**
             * @class InvalidSeedFormat
             * @brief a seed phrase is not in the expected format
             */
            class InvalidSeedFormat: DomainError {
                REGISTER_EXCEPTION(InvalidSeedFormat, -5005, "InvalidSeedFormat")
                public:
                    inline explicit InvalidSeedFormat(): DomainError("Invalid format of provided seed phrase") {};
            };

            /**
             * @class MemoryAllocationError
             * @brief Could not allocate memory
             */
            class MemoryAllocationError: ots::exception::MemoryAllocationError {
                REGISTER_EXCEPTION(MemoryAllocationError, -5006, "MemoryAllocationError")
            };

            /**
             * @class AmbigousLanguage
             * @brief Seed phrase language is ambigous. Language must be explicit specified
             */
            class AmbigousLanguage: DomainError {
                REGISTER_EXCEPTION(AmbigousLanguage, -5007, "AmbigousLanguage")
                public:
                    inline explicit AmbigousLanguage(): DomainError("Seed phrase language is ambigous. Language must be explicit specified") {};
            };

            /**
             * @class NoPasswordProvided
             * @brief Seed encrypted, but no password provided
             */
            class NoPasswordProvided: DomainError {
                REGISTER_EXCEPTION(NoPasswordProvided, -5008, "NoPasswordProvided")
                public:
                    inline explicit NoPasswordProvided(): DomainError("Seed is encrypted, but no password provided") {};
            };
        }

        /**
         * @namespace ots::exception::legacyseed
         * @brief All exceptions related to ots::LegacySeed
         * @note error codes start with -60xx
         */
        namespace legacyseed {
            class InvalidSeedFormat: ots::exception::seed::SeedDecodingFailed {
                REGISTER_EXCEPTION(InvalidSeedFormat, -6001, "InvalidSeedFormat")
                public:
                    inline explicit InvalidSeedFormat(): ots::exception::seed::SeedDecodingFailed("Invalid format of provided seed phrase") {};
            };
        }

        /**
         * @namespace ots::exception::seedjar
         * @brief All exceptions related to ots::SeedJar
         * @note error codes start with -70xx
         */
        namespace seedjar {

            /**
             * @class SeedNotFound
             * @brief a seed expected if not found
             */
            class SeedNotFound: OutOfRange {
                REGISTER_EXCEPTION(SeedNotFound, -7001, "SeedNotFound")
                public:
                    inline SeedNotFound(): OutOfRange("Seed not found") {};
            };
        }

        /**
         * @namespace ots::exception::keystore
         * @brief All exceptions related to ots::KeyStore
         * @note error codes start with -80xx
         */
        namespace keystore {
            /**
             * @class LockedAccessAttempt
             * @brief a process tried to access locked key data
             */
            class LockedAccessAttempt: SecurityException {
                REGISTER_EXCEPTION(LockedAccessAttempt, -8001, "LockedAccessAttempt")
                public:
                    inline explicit LockedAccessAttempt(): SecurityException("Attempted to access locked key data") {};
                    inline explicit LockedAccessAttempt(const std::string& msg): SecurityException(msg) {};
            };

            /**
             * @class LockedWriteAttempt
             * @brief a process tried to write to locked key data
             */
            class LockedWriteAttempt: LockedAccessAttempt {
                REGISTER_EXCEPTION(LockedWriteAttempt, -8002, "LockedWriteAttempt")
                public:
                    inline LockedWriteAttempt(): LockedAccessAttempt("Attempted to write to locked key data") {};
            };

            /**
             * @namespace ots::exception::keystore::polyseed
             * @brief All exceptions related to ots::PolyseedKeyStore
             * @note error codes start with -81xx
             */
            namespace polyseed {
                /**
                 * @class ActivePolyseedDataSession
                 * @brief a process tried to start a new session during an active session ongoing
                 */
                class ActivePolyseedDataSession: DomainError {
                    REGISTER_EXCEPTION(ActivePolyseedDataSession, -8101, "ActivePolyseedDataSession")
                    public:
                        inline explicit ActivePolyseedDataSession(): DomainError("There is already an active polyseed data session running. Only one session at a time allowed") {};
                };

                /**
                 * @class NoActivePolyseedDataSession
                 * @brief no active session ongoing which could be commited or discarded
                 */
                class NoActivePolyseedDataSession: DomainError {
                    REGISTER_EXCEPTION(NoActivePolyseedDataSession, -8102, "NoActivePolyseedDataSession")
                    public:
                        inline explicit NoActivePolyseedDataSession(): DomainError("There is no active polyseed data session running. Can't work on no existing session") {};
                };

                /**
                 * @class PolyseedDataReadOnlySession
                 * @brief write attempt while read only session
                 */
                class PolyseedDataReadOnlySession: DomainError {
                    REGISTER_EXCEPTION(PolyseedDataReadOnlySession, -8103, "PolyseedDataReadOnlySession")
                    public:
                        inline explicit PolyseedDataReadOnlySession(): DomainError("Attempt to write in read only session") {};
                };
            }
        }

        /**
         * @namespace ots::exception::wallet
         * @brief All exceptions related to ots::Wallet
         * @note error codes start with -90xx
         */
        namespace wallet {

            /**
             * @class AddressNotFound
             * @brief wallet does not contain the provided address
             */
            class AddressNotFound: OutOfRange {
                REGISTER_EXCEPTION(AddressNotFound, -9001, "AddressNotFound")
                public:
                    inline AddressNotFound(): OutOfRange("Address not found in wallet") {};
            };

            /**
             * @class ImportOutputs
             * @brief Import outputs failed
             */
            class ImportOutputs: InvalidArgument {
                REGISTER_EXCEPTION(ImportOutputs, -9002, "ImportOutputs")
                public:
                    inline explicit ImportOutputs(): InvalidArgument("Import outputs failed") {};
                    inline explicit ImportOutputs(const std::string& msg): InvalidArgument(msg) {};
            };

            /**
             * @class ExportKeyImages
             * @brief export key images failed
             */
            class ExportKeyImages: DomainError {
                REGISTER_EXCEPTION(ExportKeyImages, -9003, "ExportKeyImages")
                public:
                    inline explicit ExportKeyImages(): DomainError("Import outputs failed") {};
                    inline explicit ExportKeyImages(const std::string& msg): DomainError(msg) {};
            };

            /**
             * @class InvalidCiphertext
             * @brief the provided ciphertext is not valid
             */
            class InvalidCiphertext: DomainError {
                REGISTER_EXCEPTION(InvalidCiphertext, -9004, "InvalidCiphertext")
                public:
                    inline explicit InvalidCiphertext(): DomainError("Invalid ciphertext") {};
                    inline explicit InvalidCiphertext(const std::string& msg): DomainError(msg) {};
            };

            /**
             * @class CiphertextAuthenticationFailed
             * @brief the provided ciphertext is not valid
             */
            class CiphertextAuthenticationFailed: DomainError {
                REGISTER_EXCEPTION(CiphertextAuthenticationFailed, -9005, "CiphertextAuthenticationFailed")
                public:
                    inline explicit CiphertextAuthenticationFailed(): DomainError("Ciphertext authentication failed") {};
                    inline explicit CiphertextAuthenticationFailed(const std::string& msg): DomainError(msg) {};
            };

            /**
             * @class InternalError
             * @brief an internal error occurred (monero specific)
             */
            class InternalError: DomainError {
                REGISTER_EXCEPTION(InternalError, -9006, "InternalError")
                public:
                    inline explicit InternalError(): DomainError("Internal error") {};
                    inline explicit InternalError(const std::string& msg): DomainError(msg) {};
            };
        }

        /**
         * @namespace ots::exception::account
         * @brief All exceptions related to ots::Account
         * @note error codes start with -100xx
         */
        namespace account {
            /**
             * @class GenerationFailed
             * @brief account generation failed
             */
            class GenerationFailed: DomainError {
                REGISTER_EXCEPTION(GenerationFailed, -10001, "GenerationFailed")
                public:
                    inline explicit GenerationFailed(): DomainError("Account generation failed") {};
            };
        }

        /**
         * @namespace ots::exception::tx
         * @brief All exceptions related Transactions in ots::Wallet, ots::TxWarning, ots::TxDescription
         * @note error codes start with -110xx
         */
        namespace tx {

            /**
             * @class Invalid
             * @brief the provided unsigned transaction is not valid
             */
            class Invalid: InvalidArgument {
                REGISTER_EXCEPTION(Invalid, -11001, "Invalid")
                public:
                    inline explicit Invalid(): InvalidArgument("Transaction is invalid") {};
                    inline explicit Invalid(const std::string& msg): InvalidArgument(msg) {};
            };
        }

        /**
         * @namespace ots::exception::address
         * @brief All exceptions related to ots::Address
         * @note error codes start with -120xx
         */
        namespace address {

            /**
             * @class Invalid
             * @brief The provided string is not a valid Monero address
             */
            class Invalid: InvalidArgument {
                REGISTER_EXCEPTION(Invalid, -12001, "Invalid")
                public:
                    inline Invalid(): InvalidArgument("Not a valid Monero address") {};
            };

            /**
             * @class NotIntegrated
             * @brief The provided string is not an integrated address
             */
            class NotIntegrated: InvalidArgument {
                REGISTER_EXCEPTION(NotIntegrated, -12002, "NotIntegrated")
                public:
                    inline NotIntegrated(): InvalidArgument("Not an integrated address") {};
            };
        }
    }
}
