#include <iostream>
#include <vector>
#include <cstdlib>
#include <chrono>

using namespace std;
using namespace chrono;

void mergesort(vector<int> &a, int l, int r);
void merge_arrays(vector<int> &a, int l, int m, int r);

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cerr << "Usage: " << argv[0] << " <array_size>\n";
        return 1;
    }

    int n = atoi(argv[1]);

    // validar array
    if (n <= 0)
    {
        cerr << "array_size must be > 0\n";
        return 1;
    }

    vector<int> data(n);

    // TODO: generate random data
    srand(0); // generador de num aleatorios
    for (int i = 0; i < n; i++)
    {
        data[i] = rand();
    }

    auto t0 = high_resolution_clock::now(); // tiempo inicial

    mergesort(data, 0, n - 1);
    auto t1 = high_resolution_clock::now(); // tiempo final
    duration<double> dt = t1 - t0;
    cout << n << " " << dt.count() << "\n";

    return 0;
}

void mergesort(vector<int> &a, int l, int r)
{

    if (l >= r)
    {
        return; // si hay 0 o 1 elemento(s)
    }

    int m = l + (r - l) / 2; // encuentra el punto medio
    mergesort(a, l, m);
    mergesort(a, m + 1, r);

    merge_arrays(a, l, m, r);
}

void merge_arrays(vector<int> &a, int l, int m, int r)
{

    int n1 = m - l + 1; // izquierda
    int n2 = r - m;     // derecha

    vector<int> L(n1);
    vector<int> R(n2);

    for (int i = 0; i < n1; i++)
    {
        L[i] = a[l + i];
    }
    for (int j = 0; j < n2; j++)
    {
        R[j] = a[m + 1 + j];
    }

    int i = 0; // indice en L
    int j = 0; // indice en R
    int k = l; // indice en a

    while (i < n1 && j < n2)
    {
        if (L[i] <= R[j])
        {
            a[k] = L[i];
            i++;
        }
        else
        {
            a[k] = R[j];
            j++;
        }
        k++;
    }

    while (i < n1)
    {
        a[k] = L[i];
        i++;
        k++;
    }

    while (j < n2)
    {
        a[k] = R[j];
        j++;
        k++;
    }
}
