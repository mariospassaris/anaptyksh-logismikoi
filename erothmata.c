#include "erothmata.h"



struct Matches
{
	int id;
	int similarity_score;
};

struct array_matches
{
	ptr_matches* pinakas;
};


ptr_matches create_match(int id,int similarity)
{
	ptr_matches match = malloc(sizeof(struct Matches));

	match->id = id;
	match->similarity_score = similarity;

	return match;
}


ptr_array_matches matchSuggestion(ptr_entry n, int k, int h, int x, int limit, ptr_entity entity)    //anti gia graph pername entity
{
	int location_node;
	int i;
	ptr_entry current;
	ptr_entry node;
	ptr_graph g;
	HT_iter_ptr iter;
	ptr_array_matches array = malloc(sizeof(struct array_matches));

	array->pinakas = malloc(limit * sizeof(ptr_matches));

	g = find_graph(entity,LOCATED) ;   //na ftiaksoume find an tha einai lista h pinakas kai na arxikopoihsoume id


	if(g != NULL)
	{
		iter = HT_iter_create(g->table);

		node = lookupNode(g,n->id);     //hash
		location_node =

		for(i=0;i<g->size;i++)
		{
			current = ((ptr_entry)HT_iter_data(iter));

			HT_iter_next(iter);
		}
	}







}