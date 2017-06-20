
#include "Program.h"

#include "ssp21/crypto/Crypto.h"

#include "ssp21/crypto/gen/CertificateFile.h"
#include "ssp21/crypto/gen/CertificateEnvelope.h"
#include "ssp21/crypto/gen/CertificateBody.h"
#include "ssp21/crypto/LogMessagePrinter.h"

#include "ssp21/util/SecureFile.h"
#include "ssp21/util/SerializationUtils.h"
#include "ssp21/util/Exception.h"

#include <iostream>

#include "ConsolePrinter.h"

using namespace ssp21;

void print_contents(const std::string& path);
void print_certificate(IMessagePrinter& printer, const seq32_t& data);
void gen_x25519_key_pair(const std::string& private_key_path, const std::string& public_key_path);
void gen_ed25519_key_pair(const std::string& private_key_path, const std::string& public_key_path);
void create_certificate(const std::string& certificate_file, const std::string& private_key_path, const std::string& public_key_path);
void append(const std::string& file1, const std::string& file2, const std::string& output_file);
void calc_signature(const seq32_t& data, const CertificateFileEntry& private_key_entry, DSAOutput& signature);
PublicKeyType get_public_key_type(const CertificateFileEntry& entry);
CertificateFileEntry get_only_entry(const seq32_t& data);
void write(const std::string& path, FileEntryType type, const seq32_t& data);

template <class T>
void read_or_throw(T& item, const seq32_t& data, const std::string& path);

Program::Program() :
    parser{{
        { flags::help, { "-h", "--help" }, "shows this help message", 0 },
        { flags::show, { "-s", "--show" }, "show contents of one or more <icf files ....>", 0 },
        { flags::x25519, { "-x", "--x25519" }, "generate a x25519 key pair <private key file> <public key file>", 0 },
        { flags::ed25519, { "-d", "--ed25519" }, "generate a Ed25519 key pair <private key file> <public key file>", 0 },
        { flags::cert, { "-c", "--cert" }, "interactively generate a <certificate file> for a <public key file> signed by a <private key file>", 0 },
		{ flags::cert,{ "-a", "--append" }, "append all entries from one <icf file> and another <icf file> and write them to a destination <icf file>", 0 }
    }}
{
    if (!ssp21::Crypto::initialize())
    {
        throw std::exception("unable to initialize crypto backend");
    }
}

void Program::run(int argc, char*  argv[])
{
    const auto args = this->parser.parse(argc, argv);

    if (args[flags::help])
    {
        this->print_help();
        return;
    }

    if (args.has_option(flags::show))
    {
        for (auto& file_path : args.pos)
        {
            print_contents(file_path);
        }

        return;
    }

    if (args.has_option(flags::x25519))
    {
        if (args.pos.size() != 2)
        {
            throw std::exception("Required positional arguments: <private key file> <public key file>");
        }

        gen_x25519_key_pair(args.pos[0], args.pos[1]);
        return;
    }

    if (args.has_option(flags::ed25519))
    {
        if (args.pos.size() != 2)
        {
            throw std::exception("Required positional arguments: <private key file> <public key file>");
        }

        gen_ed25519_key_pair(args.pos[0], args.pos[1]);
        return;
    }

    if (args.has_option(flags::cert))
    {
        if (args.pos.size() != 3)
        {
            throw std::exception("Required positional arguments: <certificate file> <public key file> <private key file>");
        }

        create_certificate(args.pos[0], args.pos[1], args.pos[2]);
        return;
    }

	if (args.has_option(flags::append))
	{
		if (args.pos.size() != 3)
		{
			throw std::exception("Required positional arguments: <input icf file> <input icf file> <output icf file>");
		}

		create_certificate(args.pos[0], args.pos[1], args.pos[2]);
		return;
	}

    std::cerr << "You must specify an option" << std::endl << std::endl;
    this->print_help();
}

void Program::print_help()
{
    std::cerr << "Options: " << std::endl << std::endl;
    std::cerr << this->parser;
}

void gen_x25519_key_pair(const std::string& private_key_path, const std::string& public_key_path)
{
    std::cout << "generating x25519 key pair" << std::endl;

    KeyPair kp;
    Crypto::gen_keypair_x25519(kp);

    write(private_key_path, FileEntryType::x25519_private_key, kp.private_key.as_seq());

    write(public_key_path, FileEntryType::x25519_public_key, kp.public_key.as_seq());
}

void gen_ed25519_key_pair(const std::string& private_key_path, const std::string& public_key_path)
{
    std::cout << "generating Ed25519 key pair" << std::endl;

    KeyPair kp;
    Crypto::gen_keypair_ed25519(kp);

    write(private_key_path, FileEntryType::ed25519_private_key, kp.private_key.as_seq());

    write(public_key_path, FileEntryType::ed25519_public_key, kp.public_key.as_seq());
}

void create_certificate(const std::string& certificate_file_path, const std::string& public_key_path, const std::string& private_key_path)
{
    const auto private_key_data = SecureFile::read(private_key_path);
    const auto private_key_entry = get_only_entry(private_key_data->as_rslice());

    const auto public_key_data = SecureFile::read(public_key_path);
    const auto public_key_entry = get_only_entry(public_key_data->as_rslice());

    CertificateBody body( // TODO - interactively read these fields
        0,
        0,
        0xFFFFFFFF,
        0,
        get_public_key_type(public_key_entry),
        public_key_entry.data.take(consts::crypto::x25519_key_length)
    );

    const auto body_bytes = serialize::to_buffer(body);

    DSAOutput signature;
    calc_signature(body_bytes->as_rslice(), private_key_entry, signature);

    const auto envelope_bytes = serialize::to_buffer(
                                    CertificateEnvelope(
                                        seq32_t::empty(),
                                        signature.as_seq(),
                                        body_bytes->as_rslice().take(static_cast<uint16_t>(body_bytes->length()))
                                    )
                                );

    CertificateFile file;
    file.entries.push(
        CertificateFileEntry(
            FileEntryType::certificate,
            envelope_bytes->as_rslice()
        )
    );

    SecureFile::write(certificate_file_path, file);
}

void append(const std::string& file_path_1, const std::string& file_path_2, const std::string& output_file_path)
{
	const auto file_data_1 = SecureFile::read(file_path_1);
	CertificateFile file_1;
	read_or_throw(file_1, file_data_1->as_rslice(), file_path_1);

	const auto file_data_2 = SecureFile::read(file_path_2);
	CertificateFile file_2;
	read_or_throw(file_2, file_data_2->as_rslice(), file_path_2);
	
	CertificateFile output;
	auto add_entry = [&output](const CertificateFileEntry& entry) 
	{
		if (!output.entries.push(entry))
		{
			throw Exception("Number of total file entries exceeds capacity limit of ", output.entries.capacity());
		}
	};
	
	file_1.entries.foreach(add_entry);
	file_2.entries.foreach(add_entry);
	
	SecureFile::write(output_file_path, output);
}

template <class T>
void read_or_throw(T& item, const seq32_t& data, const std::string& path)
{
	const auto err = item.read_all(data);
	if (any(err))
	{
		throw Exception("Encountered error: ", ParseErrorSpec::to_string(err), " parsing input file: ", path);
	}
}

void calc_signature(const seq32_t& data, const CertificateFileEntry& private_key_entry, DSAOutput& signature)
{
    std::error_code ec;

    switch (private_key_entry.file_entry_type)
    {
    case(FileEntryType::ed25519_private_key):
        Crypto::sign_ed25519(data, private_key_entry.data, signature, ec);
        if (ec) throw Exception("Unable to calculate certificate siganture", ec.message());
        break;
    default:
        throw Exception("Can't produce a signature from: ", FileEntryTypeSpec::to_string(private_key_entry.file_entry_type));        
    }
}

PublicKeyType get_public_key_type(const CertificateFileEntry& entry)
{
    switch (entry.file_entry_type)
    {
    case(FileEntryType::ed25519_public_key):
        if (entry.data.length() != consts::crypto::ed25519_public_key_length) throw Exception("Bad Ed25519 public key length");
        return PublicKeyType::Ed25519;
    case(FileEntryType::x25519_public_key):
        if (entry.data.length() != consts::crypto::x25519_key_length) throw Exception("Bad x25519 public key length");
        return PublicKeyType::X25519;
    default:
        throw Exception(FileEntryTypeSpec::to_string(entry.file_entry_type), " is not a public key");
    }
}

CertificateFileEntry get_only_entry(const seq32_t& file_data)
{
    CertificateFile file;
    const auto err = file.read_all(file_data);
    if (any(err))
    {
        throw Exception("Encountered error: ", ParseErrorSpec::to_string(err), " parsing key file");
    }

    if (file.entries.count() != 1)
    {
        throw Exception("Key file contains doesn't contains a single entry, contains: ", file.entries.count());
    }

    return *file.entries.get(0);
}

void write(const std::string& path, FileEntryType type, const seq32_t& data)
{
    CertificateFile file;
    file.entries.push(CertificateFileEntry(type, data));
    SecureFile::write(path, file);
}

void print_contents(const std::string& path)
{
    const auto data = SecureFile::read(path);

    CertificateFile file;
    const auto err = file.read_all(data->as_rslice());
    if (any(err))
    {
        throw Exception("Error parsing certificate file: ", ParseErrorSpec::to_string(err));
    }

    std::cout << path << " contains " << file.entries.count() << " item(s):" << std::endl << std::endl;

    ConsolePrinter printer;

    for (auto i = 0; i < file.entries.count(); ++i)
    {
        const auto entry = file.entries.get(i);
        const auto type_name = FileEntryTypeSpec::to_string(entry->file_entry_type);

        std::cout << "item #" << i << " is a " << type_name << std::endl;

        printer.push_indent();

        if (entry->file_entry_type == FileEntryType::certificate)
        {
            print_certificate(printer, entry->data);
        }
        else
        {
            // all other types are raw keys
            printer.print("key-data", entry->data);
        }

        std::cout << std::endl;
    }
}

void print_certificate(IMessagePrinter& printer, const seq32_t& data)
{
    CertificateEnvelope envelope;

    const auto err = envelope.read_all(data);
    if (any(err))
    {
        throw Exception("Error parsing certificate envelope: ", ParseErrorSpec::to_string(err));
    }


    envelope.print("envelope", printer);

    CertificateBody body;
    const auto body_err = body.read_all(envelope.certificate_body);
    if (any(body_err))
    {
        throw Exception("Error parsing certificate body: ", ParseErrorSpec::to_string(body_err));
    }

    body.print("body", printer);
}