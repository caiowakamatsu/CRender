void exit(std::string_view reason, int error_code = 1)
{
    fmt::print("Exiting program with fatal error: {}", reason);
    exit(error_code);
}
