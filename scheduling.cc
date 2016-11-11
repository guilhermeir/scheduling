#include <glpk.h>	// glpk functions
#include <stdio.h>	// setbuf
#include <stdint.h>	// uint64_t
#include <vector>	// vector
#include <algorithm>	// fill
#include <boost/multiprecision/cpp_int.hpp>

#include "Network.h"
#include "Recursive.h"

#define M 64
#define F 100000

using namespace std;
using namespace boost::multiprecision;

int main(int argc, char** argv)
{
	setbuf(stdout,NULL);

	uint64_t n = atoi(argv[1]);
	double area = (double)atof(argv[2]);
	double tpower = 300.0;
	srand((int)area);

	Network* network = new Network(n, area, tpower);
	Recursive* recursive = new Recursive(network);
	recursive->find_fset(0);

	vector<uint64_t> sets = recursive->get_fset();
	uint64_t m = network->get_links().size();	// # of auxiliary variables (rows)
	uint64_t f = recursive->get_fset().size();	// # of structural variables	(collumns)
	uint64_t a = f * m;				// # of constraints' coefficients
	delete network;
	delete recursive;

	double z;				// object function
	int* ia = new int[1 + a]; 
	int* ja = new int[1 + a];
	double* ar = new double[1 + a];

	fill(ar, ar + 1 + a, 0);
 
	glp_prob* lp;				// glpk program instance
	lp = glp_create_prob();			// initiates lp problem
	glp_set_prob_name(lp, "scheduling");	// labels lp problem
	glp_set_obj_dir(lp, GLP_MIN);		// set lp objective direction (max or min)

	glp_add_rows(lp, m);			// auxiliary variables (constraints)
	for(uint64_t i = 0; i < m; i++)
	{
		glp_set_row_name(lp, i + 1, to_string(i).c_str());
		glp_set_row_bnds(lp, i + 1, GLP_FX, 1.0, 1.0);
	}

	glp_add_cols(lp, f);			// structural variables
	for(uint64_t i = 0; i < f; i++)
	{
		glp_set_col_name(lp, i + 1, to_string(i).c_str());
		glp_set_col_bnds(lp, i + 1, GLP_LO, 0.0, 0.0);
		glp_set_obj_coef(lp, i + 1, 1.0);
	}

	uint64_t p, q, r, i;
	for(uint64_t j = 0; j < f; j++)
	{
		p = sets[j];
		i = 0;
		do {
			q = p / 2;
			r = p % 2;
			ar[i * f + j + 1] = (r == 1) ? 1 : 0;
			p = q;
			i++;
		} while(q > 0);
	}

	for(uint64_t i = 0; i < a; i++)
        {
                ia[i + 1] = i / f + 1;
                ja[i + 1] = i % f + 1;
        }

	glp_load_matrix(lp, a, ia, ja, ar);
	glp_simplex(lp, NULL);

	double y; // if all the y values are relevant, this must be replaced with a dynamic array of size f
	z = glp_get_obj_val(lp);
	printf("z=%lf\n", z);
	for (uint64_t i = 0; i < f; i++)
	{
		y = glp_get_col_prim(lp, i + 1);
		if((y > 0) && (y < 1))
			printf("f[%ld]=%ld; y[%ld]=%lf\n", i, sets[i], i, y);
	}
	
	glp_delete_prob(lp);

	return 0;
}
