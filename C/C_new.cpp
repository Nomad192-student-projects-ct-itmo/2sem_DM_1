#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <memory>
#include <map>
#include <set>

#define NUMBER_PROBLEM "3"
#define N_CHAR_IN_WORD (100000)

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

#define MOD (1000000007)
#define N_TYPE size_t
#define N_TYPE_SP "d"
typedef N_TYPE n_type;

class DFA
{
private:
    static constexpr size_t Alphabet_size = 26;
    struct State
    {
        bool isAccept = false;
        size_t transitions[Alphabet_size]{0};
    };
    State *states;
    //size_t n;

public:
    explicit DFA(size_t n) : /*n(n),*/ states(new (nothrow) State[n + 1]) {}
    Errors check () {return (states) ? Errors::SUCCESS : Errors::ALLOCATE;}
    ~DFA() {delete[] states;}

    void change_state(size_t st, bool isAccept) {states[st].isAccept = isAccept;}
    void change_transition(size_t a, size_t b, char s) {states[a].transitions[s - 'a'] = b;}
    bool check_word(const char *word) const
    {
        size_t cur_state = 1;
        for(size_t i = 0; word[i] != '\0'; i++)
        {
            cur_state = states[cur_state].transitions[word[i] - 'a'];
            if(cur_state == 0) return false;
        }

        return states[cur_state].isAccept;
    }
    n_type n_words() const
    {
        struct Value
        {
            size_t k = 0;
            set<size_t> *was_there = nullptr;
            ~Value() {delete was_there;}
        };

        n_type result = states[1].isAccept;
        //printf("res = %d\n", result);
        auto *cur_states = new map<size_t, Value>{{1, {1, new set<size_t>{1}}}};

        while(!cur_states->empty())
        {
            auto *new_cur_states = new map<size_t, Value>;

            //printf("cur states: \n");
            for(auto cur_state : *cur_states) {
                //printf("%zu:\n", cur_state.first);
                for (int i = 0; i < Alphabet_size; i++) {
                    size_t new_state = states[cur_state.first].transitions[i];

                    if(new_state != 0) {
                        //printf("sym - %c - new - %zu\n", i + 'a', new_state);
                        if(cur_state.second.was_there->find(new_state) != cur_state.second.was_there->end()) {delete new_cur_states; delete cur_states; return -1;}
                        if(new_cur_states->find(new_state) != new_cur_states->end())
                        {
                            //printf("f\n");
                            (*new_cur_states)[new_state].k += cur_state.second.k;
                            (*new_cur_states)[new_state].was_there->insert(new_state);
                            (*new_cur_states)[new_state].was_there->insert(cur_state.second.was_there->begin(), cur_state.second.was_there->end());
                        }
                        else
                        {
                            //printf("n\n");
                            (*new_cur_states)[new_state] = {cur_state.second.k, new set<size_t>{new_state}};
                            (*new_cur_states)[new_state].was_there->insert(cur_state.second.was_there->begin(), cur_state.second.was_there->end());
                        }
                        if(states[new_state].isAccept)
                        {
                            result += cur_state.second.k;
                            printf("res += %zu - %c\n", cur_state.second.k, i + 'a');
                        }
                    }
                }
            }

            delete cur_states;
            cur_states = new_cur_states;
        }

        delete cur_states;
        return result%MOD;
    }
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

int print_answer(const n_type result)
{
    unique_ptr<FILE, int(*)(FILE*)> out(fopen(name_output_file, "w"), fclose);
    if(!out) return error_handler(Errors::OUTPUT, name_output_file);

    fprintf(out.get(), "%" N_TYPE_SP, result);

    return (int)Errors::SUCCESS;
}

int main() {
    unique_ptr<FILE, int (*)(FILE *)> in(fopen(name_input_file, "r"), fclose);
    if (!in) return error_handler(Errors::INPUT, name_input_file);

    int result = 0;
    //unique_ptr<char, void (*)(void *)> str(scan_str(in.get(), &result), free);
    //if (!str) return result;

    unique_ptr<DFA> dfa(scanDFA(in.get(), &result));
    if (!dfa)
    {
        if (result == -1)
            return print_answer("0");

        return result;
    }



//    if(dfa->n_words())
//        return print_answer("Accepts");

    return print_answer(dfa->n_words());
}