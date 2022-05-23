#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <memory>
#include <vector>
#include <set>

#define NUMBER_PROBLEM "2"
#define N_CHAR_IN_WORD (10000)

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
    STR,
    STR_FULL
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
        case Errors::STR: vprintf("Error, char in word not correct \"%c\"\n", args); break;
        case Errors::STR_FULL: vprintf("Error, word is too long\n", args); break;
    }
    va_end(args);

    return (int)e;
}

nullptr_t error_handler(enum Errors e, int *error, ...)
{
    *error = error_handler(e);
    return nullptr;
}

class NFA
{
private:
    static constexpr size_t Alphabet_size = 26;
    struct State
    {
        bool isAccept = false;
        vector<size_t> transitions[Alphabet_size];

//        State()
//        {
//            for(size_t i = 0; i < Alphabet_size; i++)
//                transitions[i].push_back(0);
//        }
    };
    State *states;

public:
    explicit NFA(size_t n) : states(new (nothrow) State[n + 1]) {}
    Errors check () {return (states) ? Errors::SUCCESS : Errors::ALLOCATE;}
    ~NFA() {delete[] states;}

    void change_state(size_t st, bool isAccept) {states[st].isAccept = isAccept;}
    void add_transition(size_t a, size_t b, char s) {states[a].transitions[s - 'a'].push_back(b);}
    bool check_word(const char *word)
    {
        auto *cur_states = new set<size_t>{1};


        for(size_t i = 0; word[i] != '\0'; i++)
        {
            if(cur_states->empty()) {delete cur_states; return false;}
            auto *new_cur_states = new set<size_t>;
            for(size_t cur_state : *cur_states)
            {
                for(size_t & k : states[cur_state].transitions[word[i] - 'a'])
                {
                    new_cur_states->insert(k);
                }
            }
            delete cur_states;
            cur_states = new_cur_states;
        }
        for(size_t cur_state : *cur_states)
            if(states[cur_state].isAccept) {delete cur_states; return true;}

        delete cur_states;
        return false;
        //return check_word_rq(0, 1, word);
    }

private:
    /*bool check_word_rq(size_t n, size_t cur_state, const char *word)
    {
        for(size_t i = n; word[i] != '\0'; i++)
        {
            if      (states[cur_state].transitions[word[i] - 'a'].empty()) return false;
            else if (states[cur_state].transitions[word[i] - 'a'].size() == 1)
                cur_state = (states[cur_state].transitions[word[i] - 'a'])[0];
            else
            {
                for(size_t j = 0; j < states[cur_state].transitions[word[i] - 'a'].size(); j++)
                {
                    if(check_word_rq(i+1, (states[cur_state].transitions[word[i] - 'a'])[j], word))
                        return true;
                }
                return false;
            }
        }
        return states[cur_state].isAccept;
    }//*/
};

char* scan_str(FILE *in, int *error)
{
    char *data = (char*)malloc((N_CHAR_IN_WORD + 1) * sizeof(char));
    if(!data) return error_handler(Errors::ALLOCATE, error);

    char buffer;
    size_t n;
    for(n = 0; fscanf(in, "%c", &buffer) != EOF && n < N_CHAR_IN_WORD; n++)
    {
        if(buffer == '\n' || buffer == ' ' || buffer == '\r' || buffer == '\t')
            break;
        if(buffer < 'a' || buffer > 'z')
        {
            free(data);
            *error = error_handler(Errors::STR, buffer);
            return nullptr;
        }
        data[n] = buffer;
    }
    if(n >= N_CHAR_IN_WORD && !(buffer == '\n' || buffer == ' ' || buffer == '\r' || buffer == '\t'))
    {
        free(data);
        *error = error_handler(Errors::STR_FULL, buffer);
        return nullptr;
    }
    data[n++] = '\0';

    //printf("%zu \"%s\"\n", n, data);

    char *new_ptr = (char*) realloc(data, n * sizeof (char));
    if(!new_ptr)
    {
        free(data);
        *error = error_handler(Errors::REALLOCATE);
        return nullptr;
    }
    return new_ptr;
}

NFA* scanDFA(FILE *in, int *error)
{
    size_t n, m, k;
    fscanf(in, "%zu %zu %zu", &n, &m, &k);
    NFA *dfa = new (nothrow) NFA(n);
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
        dfa->add_transition(a, b, symbol);
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

    unique_ptr<NFA> dfa(scanDFA(in.get(), &result));
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