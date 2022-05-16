#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <memory>

#define NUMBER_PROBLEM "1"
//#define DATA_TP unsigned int
//#define DATA_SP "u"

//typedef DATA_TP T;

constexpr char name_input_file[] = "problem" NUMBER_PROBLEM ".in";
constexpr char name_output_file[] = "problem" NUMBER_PROBLEM ".out";

using namespace std;

enum class Errors: int
{
    SUCCESS,
    INPUT,
    OUTPUT,
    ALLOCATE,
    REALLOCATE,
    //DFA_IS_NULL;
};

int error_handler(enum Errors e, ...)
{
    va_list args;
    va_start(args, e);
    switch (e) {
        case Errors::SUCCESS: break;
        case Errors::INPUT: vprintf("Error, unable to open input file \"%s\"\n", args); break;
        case Errors::OUTPUT: vprintf("Error, unable to create output file \"%s\"\n", args); break;
        case Errors::ALLOCATE: vprintf("Error, failed to allocate memory\n", args); break;
        case Errors::REALLOCATE: vprintf("Error, failed to reallocate memory\n", args); break;
        //case Errors::DFA_IS_NULL: vprintf("Rejects", args); break;
    }
    va_end(args);

    return (int)e;
}

nullptr_t error_handler(enum Errors e, int *error, ...)
{
    *error = error_handler(e);
    return nullptr;
}

class DFA
{
private:
    static constexpr size_t Alphabet_size = 26;
    struct State
    {
        bool isAccept = false;
        size_t transitions[Alphabet_size + 1] = {0};
    };
    State *states;

public:
    explicit DFA(size_t n) : states(new (nothrow) State[n + 1]) {}
    Errors check () {return (states) ? Errors::SUCCESS : Errors::ALLOCATE;}
    ~DFA() {delete[] states;}

    void change_state(size_t st, bool isAccept) {states[st].isAccept = isAccept;}
    void change_transition(size_t a, size_t b, char s) {states[a].transitions[s - 'a'] = b;}
    bool check_word(const char *word)
    {
        size_t cur_state = 1;
        for(size_t i = 0; word[i] != '\0'; i++)
        {
            cur_state = states[cur_state].transitions[word[i] - 'a'];
            if(cur_state == 0) return false;
        }

        return states[cur_state].isAccept;
    }
};

char* scan_str(FILE *in, int *error)
{
    char *data = (char*)malloc(100001 * sizeof(char));
    if(!data) return error_handler(Errors::ALLOCATE, error);

    size_t n = fscanf(in, "%100000s", data);
    char *new_ptr = (char*) realloc(data, n * sizeof (char));
    if(!new_ptr)
    {
        free(data);
        *error = error_handler(Errors::REALLOCATE);
        return nullptr;
    }
    return new_ptr;
}

DFA* scanDFA(FILE *in, int *error)
{
    size_t n, m, k;
    fscanf(in, "%zu %zu %zu", &n, &m, &k);
    DFA *dfa = new (nothrow) DFA(n);
    if(!dfa) return error_handler(Errors::ALLOCATE, error);
    if(error_handler(dfa->check()) != (int)Errors::SUCCESS)
    {
        delete dfa;
        return nullptr;
    }

    if(!k)
    {
        delete dfa;
        *error = -1;
        return nullptr;
    }
    while(k--)
    {
        size_t n_accept_state;
        fscanf(in, "%zu", &n_accept_state);
        dfa->change_state(n_accept_state, true);
    }

    while(m--)
    {
        size_t a, b;
        char symbol;
        fscanf(in, "%zu %zu %c\n", &a, &b, &symbol);
        dfa->change_transition(a, b, symbol);
    }

    return dfa;
}

int print_answer(const char *result_str)
{
    unique_ptr<FILE, int(*)(FILE*)> out(fopen(name_output_file, "w"), fclose);
    if(!out) return error_handler(Errors::OUTPUT, name_output_file);

    fprintf(out.get(), "%s", result_str);

    return (int)Errors::SUCCESS;
}

int main() {
    unique_ptr<FILE, int (*)(FILE *)> in(fopen(name_input_file, "r"), fclose);
    if (!in) return error_handler(Errors::INPUT, name_input_file);

    int result = 0;
    unique_ptr<char, void (*)(void *)> str(scan_str(in.get(), &result), free);
    if (!str) return result;

    unique_ptr<DFA> dfa(scanDFA(in.get(), &result));
    if (!dfa)
    {
        if (result == -1)
            return print_answer("Rejects");

        return result;
    }

    if(dfa->check_word(str.get()))
        return print_answer("Accepts");

    return print_answer("Rejects");
}