#pragma once

/**
 * @file ots-exceptions.hpp
 * @brief Header for the C++ library Exceptions
 */

#ifdef __PRETTY_FUNCTION__
#define NOT_IMPLEMENTED_YET() throw ots::exception::NotImplementedYet(__PRETTY_FUNCTION__)
#else
#define NOT_IMPLEMENTED_YET() throw ots::exception::NotImplementedYet(__func__)
#endif

#include <stdexcept>
#include <string>

namespace ots {
    /**
     * @namespace ots::exception
     * @brief All ots library related exceptions are in this namespace
     * @see https://en.cppreference.com/w/cpp/header/stdexcept
     */
	namespace exception {
        /**
         * @class Exception
         * @internal
         * @brief Base for all OTS exceptions with virtual what()
         */
        class Exception {
            public:
                virtual const char* what() const noexcept = 0;
                virtual ~Exception() = default;
        };

        /**
         * @class RuntimeError
         * @brief Conditions only detectable at runtime
         * @internal
         * @note Should not used directly, is to override only
         */
		class RuntimeError: std::runtime_error {
			public:
				inline explicit RuntimeError(const std::string& msg): std::runtime_error(msg) {};
		};

        /**
         * @class LogicError
         * @brief violation of logical preconditions or class invariants
         * @internal
         * @note Should not used directly, is to override only
         */
		class LogicError: std::logic_error {
			public:
				inline explicit LogicError(const std::string& msg): std::logic_error(msg) {};
		};

        /**
         * @class InvalidArgument
         * @brief provided an invalid argument
         * @internal
         * @note Should not used directly, is to override only
         */
		class InvalidArgument: std::invalid_argument {
			public:
				inline explicit InvalidArgument(const std::string& msg): std::invalid_argument(msg) {};
		};

        /**
         * @class RangeError
         * @brief range error in internal computation
         * @note Should not used directly, is to override only
         */
		class RangeError: std::range_error {
			public:
				inline explicit RangeError(const std::string& msg): std::range_error(msg) {};
		};

        /**
         * @class NotImplementedYet
         * @brief Sole purpose of this exception is to create already the stub and mark as incomplete, you should never get such exception in the complete library
         */
		class NotImplementedYet: std::logic_error {
			public:
				inline NotImplementedYet(): std::logic_error("Not implemented yet") {};
				inline explicit NotImplementedYet(const std::string& msg): std::logic_error(std::string("Not implemented yet: ") + msg) {};
		};

        /**
         * @class DomainError
         * @brief errors related to the library use itself
         * @note Should not used directly, is to override only
         */
		class DomainError: std::domain_error {
			public:
				inline explicit DomainError(const std::string& msg): std::domain_error(msg) {};
		};

        /**
         * @class OutOfRange
         * @brief arguments outside of expected range. Needle not in Haystack ;)
         * @internal
         * @note Should not used directly, is to override only
         */
		class OutOfRange: std::out_of_range {
			public:
				inline explicit OutOfRange(const std::string& msg): std::out_of_range(msg) {};
		};

        /**
         * @class BufferOverflowException
         * @brief size is bigger then buffer
         */
		class BufferOverflowException: OutOfRange {
			public:
				inline explicit BufferOverflowException(): OutOfRange("The size is bigger as the buffer! Security related exception!") {};
				inline explicit BufferOverflowException(const std::string& msg): OutOfRange(msg) {};
		};

        /**
         * @class MemoryAllocationError
         * @brief Could not allocate memory
         */
        class MemoryAllocationError: RuntimeError {
			public:
				inline explicit MemoryAllocationError(): RuntimeError("Couldn't allocate memory!") {};
        };

        /**
         * @class SecurityException
         * @brief Any security realted issue
         */
        class SecurityException: DomainError {
			public:
				inline explicit SecurityException(const std::string& msg): DomainError(std::string("SECURITY EXCEPTION: ") + msg) {};
        };

        /**
         * @class SecurityError
         * @brief Any security realted error: like no random generator available
         */
        class SecurityError: RuntimeError {
            public:
                inline explicit SecurityError(const std::string& msg): RuntimeError(std::string("SECURITY ERROR: ") + msg) {};
        };

        /**
         * @class LowEntropy
         * @brief Low entropy detected
         */
        class LowEntropy: SecurityError {
            public:
                inline explicit LowEntropy(): SecurityError("Low entropy detected!") {};
        };

        /**
         * @class UnknownNetwork
         * @brief a network is not known
         */
        class UnknownNetwork: DomainError {
            public:
                inline explicit UnknownNetwork(): DomainError("Unknown network") {};
        };

        /**
         * @namespace ots::exception::wipeablestring
         * @brief All exceptions related to ots::WipeableString
         */
        namespace wipeablestring {

            /**
             * @class UnsafeConversion
             * @brief a conversion to std::string is not allowed
             */
            class UnsafeConversion: SecurityException {
                public:
                    inline explicit UnsafeConversion(): SecurityException("Unsafe conversion to std::string, use wipeableString.insecure() instead, if you really need to do this!") {};
            };
        }

        /**
         * @namespace ots::exception::wipeablevector
         * @brief All exceptions related to ots::WipeableVector
         */
        namespace wipeablevector {
            class UnsafeConversion: SecurityException {
                public:
                    /**
                     * @brief a conversion to std::vector is not allowed
                     */
                    inline explicit UnsafeConversion(): SecurityException("Unsafe conversion to std::vector, use wipeableVector.insecure() instead, if you really need to do this!") {};
            };
        }

        namespace sign {
            class InvalidSignature: DomainError {
                public:
                    inline explicit InvalidSignature(): DomainError("Invalid signature") {};
                    inline explicit InvalidSignature(const std::string& msg): DomainError(msg) {};
            };

            class EmptyMessage: DomainError {
                public:
                    inline explicit EmptyMessage(): DomainError("Empty message") {};
            };

            class EmptySignature: DomainError {
                public:
                    inline explicit EmptySignature(): DomainError("Empty signature") {};
            };
        }

        /**
         * @namespace ots::exception::seed
         * @brief All exceptions related to ots::Seed
         */
		namespace seed {
            /**
             * @class NoDefaultLanguageSet
             * @brief a default language is expected but not set
             */
			class NoDefaultLanguageSet: DomainError {
				public:
				inline explicit NoDefaultLanguageSet(): DomainError("No default language set for seed type") {};
			};

            /**
             * @class LanguageNotFound
             * @brief a language expected if not found
             */
			class LanguageNotFound: OutOfRange {
				public:
					inline explicit LanguageNotFound(): OutOfRange("Language not found") {};
					inline explicit LanguageNotFound(const std::string& msg): OutOfRange(msg) {};
			};

            /**
             * @class LanguageNotSupportedBySeedType
             * @brief a language is not supported by the seed type
             */
            class LanguageNotSupportedBySeedType: DomainError {
                public:
                    inline explicit LanguageNotSupportedBySeedType(): DomainError("Language not supported by seed type") {};
            };

            /**
             * @class WordCount
             * @brief a seed expected has a different word count. Polyseed are 16 words, Monero Seeds are 25 words, Legacy Seeds are 13 words
             */
            class WordCount: OutOfRange {
				public:
					inline explicit WordCount(): OutOfRange("Wrong number of words in seed phrase") {};
            };

            /**
             * @class SeedEncodingFailed
             * @brief a seed could not be encoded
             */
            class SeedEncodingFailed: DomainError {
				public:
                    inline explicit SeedEncodingFailed(): DomainError("Unable to encode seed") {};
                    inline explicit SeedEncodingFailed(const std::string& msg): DomainError(msg) {};
            };

            /**
             * @class SeedDecodingFailed
             * @brief a seed could not be encoded
             */
            class SeedDecodingFailed: DomainError {
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
                public:
                    inline explicit LengthMismatch(): DomainError("Seed length mismatch") {};
            };

            /**
             * @class TooFewValues
             * @brief you need at least two values to merge.
             */
            class TooFewValues: DomainError {
                public:
                    inline explicit TooFewValues(): DomainError("Too few values provided") {};
            };

            /**
             * @class PasswordNotSupported
             * @brief a password is not supported for the seed type
             */
            class PasswordNotSupported: DomainError {
                public:
                    inline explicit PasswordNotSupported(): DomainError("Password not supported for seed type") {};
            };

            /**
             * @class MergeError
             * @brief a seed index could not be merged
             */
            class MergeError : public DomainError {
                public:
                    explicit MergeError(const std::string& msg) : DomainError(msg) {}
            };
		}

        namespace polyseed {
            class WordCount: ots::exception::seed::WordCount {};
            class UnsupportedLanguage: ots::exception::seed::LanguageNotFound {
                public:
					inline explicit UnsupportedLanguage(): ots::exception::seed::LanguageNotFound("Unsupported Language") {};
            };

            class ChecksumMismatch: DomainError {
				public:
				inline explicit ChecksumMismatch(): DomainError("Checksum mismatch") {};
            };
            class UnsupportedFeatures: DomainError {
				public:
				inline explicit UnsupportedFeatures(): DomainError("One or more features are not supported") {};
            };
            class InvalidSeedFormat: DomainError {
				public:
				inline explicit InvalidSeedFormat(): DomainError("Invalid format of provided seed phrase") {};
            };
            class MemoryAllocationError: ots::exception::MemoryAllocationError {};
            class AmbigousLanguage: DomainError {
				public:
				inline explicit AmbigousLanguage(): DomainError("Seed phrase language is ambigous. Language must be explicit specified") {};
            };

            /**
             * @brief Seed encrypted, but no password provided
             */
            class NoPasswordProvided: DomainError {
                public:
                    inline explicit NoPasswordProvided(): DomainError("Seed is encrypted, but no password provided") {};
            };
        }

        namespace legacyseed {
            class InvalidSeedFormat: ots::exception::seed::SeedDecodingFailed {
				public:
				inline explicit InvalidSeedFormat(): ots::exception::seed::SeedDecodingFailed("Invalid format of provided seed phrase") {};
            };
        }

        /**
         * @namespace ots::exception::seedjar
         * @brief All exceptions related to ots::SeedJar
         */
        namespace seedjar {

            /**
             * @class SeedNotFound
             * @brief a seed expected if not found
             */
			class SeedNotFound: OutOfRange {
				public:
					inline SeedNotFound(): OutOfRange("Seed not found") {};
			};
        }

        /**
         * @namespace ots::exception::keystore
         * @brief All exceptions related to ots::KeyStore
         */
        namespace keystore {
            /**
             * @brief a process tried to access locked key data
             */
            class LockedAccessAttempt: SecurityException {
                public:
                    inline explicit LockedAccessAttempt(): SecurityException("Attempted to access locked key data") {};
                    inline explicit LockedAccessAttempt(const std::string& msg): SecurityException(msg) {};
            };

            /**
             * @brief a process tried to write to locked key data
             */
            class LockedWriteAttempt: LockedAccessAttempt {
                public:
                    inline LockedWriteAttempt(): LockedAccessAttempt("Attempted to write to locked key data") {};
            };

            /**
             * @namespace ots::exception::keystore::polyseed
             * @brief All exceptions related to ots::PolyseedKeyStore
             */
            namespace polyseed {
                /**
                 * @brief a process tried to start a new session during an active session ongoing
                 */
                class ActivePolyseedDataSession: DomainError {
                    public:
                        inline explicit ActivePolyseedDataSession(): DomainError("There is already an active polyseed data session running. Only one session at a time allowed") {};
                };

                /**
                 * @brief no active session ongoing which could be commited or discarded
                 */
                class NoActivePolyseedDataSession: DomainError {
                    public:
                        inline explicit NoActivePolyseedDataSession(): DomainError("There is no active polyseed data session running. Can't work on no existing session") {};
                };

                /**
                 * @brief write attempt while read only session
                 */
                class PolyseedDataReadOnlySession: DomainError {
                    public:
                        inline explicit PolyseedDataReadOnlySession(): DomainError("Attempt to write in read only session") {};
                };
            }
        }

        /**
         * @namespace ots::exception::wallet
         * @brief All exceptions related to ots::Wallet
         */
		namespace wallet {

            /**
             * @class AddressNotFound
             * @brief wallet does not contain the provided address
             */
			class AddressNotFound: OutOfRange {
				public:
					inline AddressNotFound(): OutOfRange("Address not found in wallet") {};
			};

            /**
             * @class ImportOutputs
             * @brief Import outputs failed
             */
            class ImportOutputs: InvalidArgument {
				public:
					inline explicit ImportOutputs(): InvalidArgument("Import outputs failed") {};
					inline explicit ImportOutputs(const std::string& msg): InvalidArgument(msg) {};
            };

            /**
             * @class ExportKeyImages
             * @brief export key images failed
             */
            class ExportKeyImages: DomainError {
				public:
					inline explicit ExportKeyImages(): DomainError("Import outputs failed") {};
					inline explicit ExportKeyImages(const std::string& msg): DomainError(msg) {};
            };
		}

        namespace account {
            /**
             * @class GenerationFailed
             * @brief account generation failed
             */
            class GenerationFailed: DomainError {
                public:
                    inline explicit GenerationFailed(): DomainError("Account generation failed") {};
            };
        }

        /**
         * @namespace ots::exception::tx
         * @brief All exceptions related Transactions in ots::Wallet, ots::TxWarning, ots::TxDescription
         */
		namespace tx {

            /**
             * @class Invalid
             * @brief the provided unsigned transaction is not valid
             */
            class Invalid: InvalidArgument {
				public:
					inline explicit Invalid(): InvalidArgument("Transaction is invalid") {};
					inline explicit Invalid(const std::string& msg): InvalidArgument(msg) {};
            };
		}

        /**
         * @namespace ots::exception::address
         * @brief All exceptions related to ots::Address
         */
		namespace address {

            /**
             * @class Invalid
             * @brief The provided string is not a valid Monero address
             */
			class Invalid: InvalidArgument {
				public:
					inline Invalid(): InvalidArgument("Not a valid Monero address") {};
			};

            /**
             * @class NotIntegrated
             * @brief The provided string is not an integrated address
             */
			class NotIntegrated: InvalidArgument {
				public:
					inline NotIntegrated(): InvalidArgument("Not an integrated address") {};
			};
		}
	}
}
