#include "queries.h"


struct Matches
{
	int id;
	double similarity_score;
};

struct array_matches
{
	int current_size;
	int limit;
	ptr_matches* pinakas;
};


ptr_matches create_match(int id,double similarity)
{
	ptr_matches match = malloc(sizeof(struct Matches));

	match->id = id;
	match->similarity_score = similarity;

	return match;
}

void delete_match(ptr_matches match)
{
	free(match);
}

void insert_match (ptr_array_matches array,ptr_matches match)
{

	if( (array->current_size) < (array->limit) )
	{
		array->current_size++;
		array->pinakas[array->current_size] = match;

		qsort (array->pinakas, ((array->current_size)+1), sizeof(struct Matches *), compare_match);
	}
	else if( (array->current_size) == (array->limit) )
	{
		if(compare_match(&(array->pinakas[array->limit]),&match) > 0 )
		{
			delete_match(array->pinakas[array->limit]);
			array->pinakas[array->limit] = match;

			qsort (array->pinakas, ((array->current_size)+1), sizeof(struct Matches *), compare_match);
		}
	}


}

int compare_match (const void * a, const void * b)
{
	ptr_matches* match1 = ((ptr_matches*) a);
	ptr_matches* match2 = ((ptr_matches*) b);

	int simil1 = (*match1)->similarity_score;
	int simil2 = (*match2)->similarity_score;

	return ( simil2 - simil1 );

}


ptr_array_matches create_array_match(int limit)
{
	ptr_array_matches array = malloc(sizeof(struct array_matches));

	array->pinakas = malloc(limit * sizeof(struct Matches *));
	array->current_size = -1;
	array->limit = limit - 1;

	return array;
}


void delete_array_matches(ptr_array_matches array)
{
	int limit = array->current_size;
	int i;
	ptr_matches match;

	for(i=0;i<limit;i++)
	{
		//printf("only one\n");
		delete_match((array->pinakas[i]));
	}
	free(array);
}

int get_match(int pos,ptr_array_matches array,double *score)
{
	if(pos > array->current_size)
	{
		printf("Wrong pos !!!\n");
		return -1;
	}
	printf("In get_match\n");
	printf("ID = %d\n",(array->pinakas[pos])->id);

	*score = ((array->pinakas[pos])->similarity_score);
	return ((array->pinakas[pos])->id);
}


ptr_array_matches matchSuggestion(ptr_entry node, int commonInterest, int hops, int ageDiff, int limit, ptr_graph graph)
{
	int location_node = location_of_entry(node);
	int work = last_work_or_study_of_entry(node,"person_workAt_organisation.csv");
	int study = last_work_or_study_of_entry(node,"person_studyAt_organisation.csv");
	int koinaEndiaf;
	int node_interest;
	int data_interest;
	int gap;
	int same_sex;
	int apostash;

	int location_node2;
	int work2;
	int study2;

	int i, graph_size = Graph_size(graph);
	HT_iter_ptr iter;
	ht_ptr nodes = Graph_nodes(graph);
	ptr_entry data;

	ptr_array_matches array = create_array_match(limit);
	ptr_matches match;
	double similarity = 0.0;

	iter = HT_iter_create(nodes);
	//printf("In matchSuggestion\n");

	for(i=0;i<graph_size;i++)
	{
		data = ((ptr_entry)HT_iter_data(iter));
		if(data->id == node->id)
		{
			HT_iter_next(iter);
			continue;
		}
		//printf("Node id = %d and Data id = %d\n",node->id,data->id);

		location_node2 = location_of_entry(data);
		//printf("location = %d\n",location_node2);
		work2 = last_work_or_study_of_entry(data,"person_workAt_organisation.csv");
		//printf("work = %d\n",work2);
		study2 = last_work_or_study_of_entry(data,"person_studyAt_organisation.csv");
		//printf("study = %d\n",study2);
		koinaEndiaf = common_interests_two_entries(node,data,&node_interest,&data_interest);
		//printf("koinaEndiaf = %d and commonInt = %d\n",koinaEndiaf,commonInterest);
		gap = generation_gap(node,data);
		//printf("gap\n");
		same_sex = same_gender(node,data);
		//printf("same_sex\n");
		apostash = 1;//reachNode1(graph,node->id,data->id);
		//printf("apostash\n");

		if( ((location_node == location_node2) || (study == study2) || (work == work2))
			&& (koinaEndiaf >= commonInterest)
			&& (gap <= ageDiff)
			&& (!same_sex)
			&& (apostash <= hops) )
		{
			//printf("*****insert one match data id = %d\n",data->id);
			similarity = ((double)koinaEndiaf)/((double)(node_interest + data_interest));
			match = create_match(data->id,similarity);

			insert_match(array,match);
		}

		HT_iter_next(iter);
	}

	HT_iter_destroy(iter);

	return array;
}

/************************ Stalkers ****************************/

ptr_graph Create_Stalkers_Graph(int stalkersNum,int likesNumber,int centralityMode,ptr_database database)
{
	ptr_graph graph = DB_get_entity(database,PERSON);
	ptr_graph post_graph = DB_get_entity(database,POST);
	ptr_graph forum_graph = DB_get_entity(database,FORUM);


	int i, graph_size = Graph_size(graph);
	int j;
	HT_iter_ptr iter;
	ht_ptr nodes = Graph_nodes(graph);
	ptr_entry data;

	ptr_graph stalker_graph = createGraph(PERSON,TABLE_DEFAULT_SIZE, BUCKET_DEFAULT_SIZE);
	ptr_entry entry_stalker;
	ptr_edge edge_stalker;


	list_ptr person_like_post_list;
	LL_iter_ptr iterList;
	int list_size;
	ptr_edge edge;
	int post_id;
	ptr_entry post_entry;
	int id_creator_of_post;
	ptr_katanomh dataKat;
	list_ptr list = NULL;
	LL_iter_ptr iterCreator;
	int bhmata;
	int id_creat;
	int id_forum;
	ptr_edge stalker_edge;
	int err = 8;



	iter = HT_iter_create(nodes);
	//printf("In getTopStalkers\n");

	for(i=0;i<graph_size;i++)
	{
		data = ((ptr_entry)HT_iter_data(iter));
		//printf("Data id = %d\n",data->id);

		person_like_post_list = type_list(data,"person_likes_post.csv");


		if(person_like_post_list != NULL)
		{
			list = LL_create(match_friend);
			list_size = LL_size(person_like_post_list);
			//printf("If list_size_of_person_like_post = %d\n",list_size);
			iterList = LL_iter_create(person_like_post_list);
			//printf("If list_size_of_person_like_post > 0\n");
			for(j=0;j<list_size;j++)
			{
				edge = ((ptr_edge)LL_iter_data(iterList));
				post_id = edge->target_id;

				post_entry = lookupNode(post_graph,post_id);

				if(post_entry != NULL)
				{
					//printf("post_id = %d\n",post_id);
					id_creator_of_post = creator_of_post(post_entry);
					if(id_creator_of_post != -1)
					{
						//printf("id_creator = %d\n",id_creator_of_post);
						manage_list(list,id_creator_of_post);
					}
				}

				LL_iter_next(iterList);
			}

			//printf("If list_size_of_person_like_post > 0\n");
			if(LL_size(list) > 0)
			{
				//printf("If list_size_of_creator_ids = %d\n",LL_size(list));

				iterCreator = LL_iter_create(list);

				for(j=0;j<LL_size(list);j++)
				{
					dataKat = (ptr_katanomh)LL_iter_data(iterCreator);
					id_creat = katanomh_get_id(dataKat);
					bhmata = reachNode1(graph,data->id,id_creat);

					if((katanomh_get_size(dataKat) > likesNumber) && (bhmata > 1))
					{
						//printf("Find a stalker %d\n",data->id);
						//meta na kataskeuaso ena graph stalker me entries kai edges forums pou anhkei
						entry_stalker = copy_entry_person_knows_person(stalker_graph,data);
						//printf("TRY to Insert stalker %d\n",data->id);
						//insertNode(stalker_graph,entry_stalker);

						isMemberInThisForum(forum_graph,stalker_graph,entry_stalker->id);

						//printf("Insert stalker %d\n",data->id);

						break;
					}

					LL_iter_next(iterCreator);
				}
			}

			LL_destroy(list,destroy_data);

		}

		HT_iter_next(iter);
	}

	return stalker_graph;

}


ptr_graph getTopStalkers(int stalkersNum,int likesNumber,int centralityMode,ptr_database database,ptr_array_matches stalkersCloseCentr)
{

	ptr_matches stalker;
	double centrality = 0.0;
	//stalkersCloseCentr = create_array_match(stalkersNum);

	ptr_graph stalker_graph = Create_Stalkers_Graph(stalkersNum,likesNumber,centralityMode,database);

	HT_iter_ptr iter;
	ht_ptr nodes = Graph_nodes(stalker_graph);
	ptr_entry data;
	int i, graph_size = Graph_size(stalker_graph);
	int j;



	iter = HT_iter_create(nodes);
	//printf("In getTopStalkers\n");

	for(i=0;i<graph_size;i++)
	{
		data = ((ptr_entry)HT_iter_data(iter));
		//printf("Data id = %d\n",data->id);

		///tote o data entry einai stalker
		if(centralityMode == 1) centrality = closeness_centrality(data,stalker_graph);
		else if(centralityMode == 2) centrality = betweenness_centrality(data,stalker_graph);

		stalker = create_match(data->id,centrality);

		insert_match(stalkersCloseCentr,stalker);

		HT_iter_next(iter);
	}

	return stalker_graph;

}




/************************* findTrends ***************************/


struct trend
{
	int size;
	char *tag_name;
};

struct array_trends
{
	int current_size;
	int limit;
	ptr_trend* pinakas;
};


ptr_trend create_trend(int size,char* tag_name)
{
	ptr_trend trend = malloc(sizeof(struct trend));

	trend->size = size;
	trend->tag_name = strdup(tag_name);
	//printf("CREATE TREND tag_name = %s\n",trend->tag_name);

	return trend;
}

void delete_trend(ptr_trend trend)
{
	free(trend->tag_name);
	free(trend);
}

void insert_trend (ptr_array_trends array,ptr_trend trend)
{
	int i;
	if( (array->current_size) < (array->limit) )
	{

		array->current_size++;
		//printf("current size = %d\n",array->current_size);

		array->pinakas[array->current_size] = trend;
		//printf("array trend name = %s and trend name = %s\n",(array->pinakas[array->current_size])->tag_name,trend->tag_name);
		//printf("current = %d\n",array->current_size);

		qsort (array->pinakas, ((array->current_size)+1), sizeof(struct trend *), compare_trend);

		//printf("array trend name = %s and trend name = %s\n",(array->pinakas[array->current_size])->tag_name,trend->tag_name);
	}
	else if( (array->current_size) == (array->limit) )
	{
		if(compare_trend(&(array->pinakas[array->limit]),&trend) > 0 )
		{
			delete_trend(array->pinakas[array->limit]);
			array->pinakas[array->limit] = trend;
			//printf("array trend name = %s and trend name = %s\n",(array->pinakas[array->current_size])->tag_name,trend->tag_name);

			qsort (array->pinakas, ((array->current_size)+1), sizeof(struct trend *), compare_trend);

			//printf("array trend name = %s and trend name = %s\n",(array->pinakas[array->current_size])->tag_name,trend->tag_name);
		}
	}
}

int compare_trend (const void * a, const void * b)
{
	ptr_trend* trend1 = ((ptr_trend*) a);
	ptr_trend* trend2 = ((ptr_trend*) b);

	int size1 = (*trend1)->size;
	int size2 = (*trend2)->size;

	return ( size2 - size1 );
}


ptr_array_trends create_array_trends(int limit)
{
	ptr_array_trends array = malloc(sizeof(struct array_trends));

	array->pinakas = malloc(limit * sizeof(struct trend *));
	array->current_size = -1;
	array->limit = limit-1;

	return array;
}


void delete_array_trends(ptr_array_trends array)
{
	int limit = array->current_size;
	int i;
	ptr_trend trend;

	for(i=0;i<limit;i++)
	{
		//printf("only one\n");
		delete_trend((array->pinakas[i]));
	}
	free(array);
}



char* get_trend_name(int pos,ptr_array_trends array,int *size)
{
	if(pos > array->current_size)
	{
		printf("Wrong pos !!!\n");
		return "wrong Pos";
	}
	//printf("In get_match\n");
	//printf("ID = %d\n",(array->pinakas[pos])->id);

	*size = ((array->pinakas[pos])->size);
	//printf("tag_name in get trend = %s\n",(array->pinakas[pos])->tag_name);
	return ((array->pinakas[pos])->tag_name);
}





void findTrends(int trendsNum,ptr_database database,char** womenTrends,char** menTrends)
{
	ptr_graph graph = DB_get_entity(database,PERSON);
	ptr_graph post_graph = DB_get_entity(database,POST);
	ptr_graph forum_graph = DB_get_entity(database,FORUM);
	ptr_graph tag_graph = DB_get_entity(database,TAG);

	ptr_graph women;
	ptr_graph men;


	HT_iter_ptr iter;
	ht_ptr nodes = Graph_nodes(tag_graph);
	ptr_entry data_tag;
	int i, graph_size = Graph_size(tag_graph);
	int j;

	int numCCwomen;
	int maxCCwomen;
	int numCCmen;
	int maxCCmen;

	ptr_array_trends women_trends = create_array_trends(trendsNum);
	ptr_array_trends men_trends = create_array_trends(trendsNum);

	ptr_trend trend_women;
	ptr_trend trend_men;

	ptr_tag_info tag_info;
	char *tag_name;
	char *tag_name1;
	char *tag_name2;
	int size_tag;
	int women_size;
	int men_size;


	iter = HT_iter_create(nodes);
	//printf("In getTopStalkers\n");

	for(i=0;i<graph_size;i++)
	{
		data_tag = ((ptr_entry)HT_iter_data(iter));
		//printf("Data id = %d\n",data_tag->id);

		women = createGraph(PERSON,TABLE_DEFAULT_SIZE, BUCKET_DEFAULT_SIZE);
		men = createGraph(PERSON,TABLE_DEFAULT_SIZE, BUCKET_DEFAULT_SIZE);

		personHasInterestTag(graph,women,men,data_tag->id);

		//printf("\n\n^^^^^^^^^^^^ WOMEN graph ^^^^^^^^^^^^\n\n");
		//print_graph(women);
		//printf("\n\n^^^^^^^^^^^^ MEN graph ^^^^^^^^^^^^\n\n");
		//print_graph(men);

		women_size = Graph_size(women);
		men_size = Graph_size(men);

		if(women_size > 0) numCCwomen = numberOfCCs(women);
		else numCCwomen = 0;
		if(men_size > 0) numCCmen = numberOfCCs(men);
		else numCCmen = 0;
		//printf("************ numCCwomen = %d   numCCmen = %d **********\n",numCCwomen,numCCmen);

		if(women_size > 0) maxCCwomen = maxCC(women);
		else maxCCwomen = 0;
		if(men_size > 0) maxCCmen = maxCC(men);
		else maxCCmen = 0;
		//printf("************ maxCCwomen = %d   maxCCmen = %d **********\n",maxCCwomen,maxCCmen);

		tag_info = data_tag->properties;
		tag_name = get_tag_name(tag_info);
		//printf("tag_name = %s\n",tag_name);
		tag_name1 = strdup(tag_name);
		tag_name2 = strdup(tag_name);
		//printf("tag_name1 = %s\n",tag_name1);
		//printf("tag_name2 = %s\n",tag_name2);

		trend_women = create_trend(maxCCwomen,tag_name1);
		trend_men = create_trend(maxCCmen,tag_name2);

		insert_trend(women_trends,trend_women);
		insert_trend(men_trends,trend_men);


		destroyGraph(women);
		destroyGraph(men);

		free(tag_name1);
		free(tag_name2);

		HT_iter_next(iter);
	}

	HT_iter_destroy(iter);

	for(i=0;i<=(women_trends->current_size);i++)
	{
		tag_name = get_trend_name(i,women_trends,&size_tag);
		//printf("***** WOMEN[%d] tag_name = %s  and  size_tag = %d\n",i,tag_name,size_tag);
		womenTrends[i] = strdup(tag_name);
	}

	for(i=0;i<=(men_trends->current_size);i++)
	{
		tag_name = get_trend_name(i,men_trends,&size_tag);
		//printf("+++++ MEN[%d] tag_name = %s  and  size_tag = %d\n",i,tag_name,size_tag);
		menTrends[i] = strdup(tag_name);
	}

	return;

}






/***************** trust graph and estimate ****************/

double trust(ptr_entry node_i,ptr_entry node_j,ptr_graph graph,ptr_graph post_graph,ptr_graph comment_graph)
{

	int i, graph_size = Graph_size(graph);
	int j;
	HT_iter_ptr iter;
	ht_ptr nodes = Graph_nodes(graph);
	ptr_entry data;


	list_ptr person_like_post_list;
	LL_iter_ptr iterList;
	int list_size = 0;
	ptr_edge edge;
	int post_id;
	ptr_entry post_entry;
	int id_creator_of_post;
	ptr_katanomh dataKat;
	list_ptr list = NULL;
	LL_iter_ptr iterCreator;
	int bhmata;
	int id_creat;
	int id_forum;
	ptr_edge stalker_edge;
	int err = 8;

	int countIlikeJ = 0;

	list_ptr list_posts_node_j;
	list_ptr list_comments_node_i;

	//printf("node_i = %d  node_j = %d\n",node_i->id,node_j->id);

	person_like_post_list = type_list(node_i,"person_likes_post.csv");


	if(person_like_post_list != NULL)
	{
		list_size = LL_size(person_like_post_list);
		//printf("If list_size_of_person_like_post = %d\n",list_size);
		iterList = LL_iter_create(person_like_post_list);
		//printf("If list_size_of_person_like_post > 0\n");
		for(j=0;j<list_size;j++)
		{
			edge = ((ptr_edge)LL_iter_data(iterList));
			post_id = edge->target_id;
			//printf("&&&&&& like post id = %d\n",post_id);

			post_entry = lookupNode(post_graph,post_id);

			if(post_entry != NULL)
			{
				//printf("***** post_id = %d\n",post_id);
				id_creator_of_post = creator_of_post(post_entry);
				if(id_creator_of_post == node_j->id)
				{
					//printf("***** id_creator = %d\n",id_creator_of_post);
					countIlikeJ = countIlikeJ + 1;
				}
			}

			LL_iter_next(iterList);
		}
		LL_iter_destroy(iterList);
	}


	///gia replies menei

	list_posts_node_j = list_of_post_from_person_node(node_j,post_graph);
	list_comments_node_i = list_of_comments_from_person_node(node_i,comment_graph);

	LL_iter_ptr iter_com;
	int list_com_size = LL_size(list_comments_node_i);
	int list_post_size = LL_size(list_posts_node_j);
	ptr_entry comment;
	list_ptr list_comment_reply;
	LL_iter_ptr iter_reply;
	int reply_size = 0;
	int reply_size_all = 0;
	ptr_edge edge_reply;
	ptr_entry new;
	ptr_entry new1 = NULL;

	int countReplies = 0;

	if(list_com_size != 0 && list_post_size != 0)
	{
		iter_com = LL_iter_create(list_comments_node_i);

		for(i=0;i<list_com_size;i++)
		{
			comment = ((ptr_entry) LL_iter_data(iter_com));
			//printf("*****comment id = %d\n",comment->id);

			list_comment_reply = type_list(comment,"comment_replyOf_post.csv");
			if(list_comment_reply != NULL)
			{
				reply_size = LL_size(list_comment_reply);
				reply_size_all = reply_size_all + reply_size;
				iter_reply = LL_iter_create(list_comment_reply);
				for(j=0;j<reply_size;j++)
				{
					edge_reply = LL_iter_data(iter_reply);
					//printf("******reply to post %d\n",edge_reply->target_id);

					new = create_entry(edge_reply->target_id,NULL,NULL);
					new1 = LL_search(list_posts_node_j,((void *) new));
					if(new1 != NULL)
					{
						//printf("^^^^^^ find this post %d in list_post_node_j\n",edge_reply->target_id);
						countReplies = countReplies + 1;
					}
					destroy_entry(((void *) new));

					LL_iter_next(iter_reply);
				}
				LL_iter_destroy(iter_reply);
			}


			LL_iter_next(iter_com);
		}
		LL_iter_destroy(iter_com);
	}

	//printf("countLike = %d   and   countReply = %d\n",countIlikeJ,countReplies);
	//printf("listSize = %d   and   reply_size_all = %d\n",list_size,reply_size_all);

	double apotel = 0.0;
	double first = 0.0;
	double second = 0.0;
	if(list_size != 0 )
	{
		first = ( 0.3*(((double)countIlikeJ)/((double)list_size)) );
		//printf("**** first = %f\n",first);
	}
	if(reply_size_all != 0)
	{
		second = ( 0.7*(((double)countReplies)/((double) reply_size_all)) );
		//printf("**** second = %f\n",second);
	}
	apotel = first + second;
	//printf("apotel = %f\n",apotel);

	return apotel;

}


ptr_graph buildTrustGraph(int forumID,ptr_database database)
{
	ptr_graph graph = DB_get_entity(database,PERSON);
	ptr_graph post_graph = DB_get_entity(database,POST);
	ptr_graph forum_graph = DB_get_entity(database,FORUM);
	ptr_graph comment_graph = DB_get_entity(database,COMMENT);


/*
	int trustANodeId = 30;
	int trustBNodeId = 9805;
	int trustCNodeId = 9700;
	ptr_entry ta = lookupNode(graph,trustANodeId);
	ptr_entry tb = lookupNode(graph,trustBNodeId);
	ptr_entry tc = lookupNode(graph,trustCNodeId);

	trust(ta,tb,graph,post_graph,comment_graph);
*/

	ptr_graph trust_graph = createGraph(PERSON,TABLE_DEFAULT_SIZE, BUCKET_DEFAULT_SIZE);

	ptr_entry forum_entry = lookupNode(forum_graph,forumID);
	list_ptr list_of_members = type_list(forum_entry,"forum_hasMember_person.csv");

	LL_iter_ptr iter;
	int list_size;
	int i;
	ptr_edge edge;
	ptr_entry person_entry;

	if(list_of_members != NULL)
	{
		iter = LL_iter_create(list_of_members);
		list_size = LL_size(list_of_members);
		for(i=0;i<list_size;i++)
		{
			edge = LL_iter_data(iter);

			person_entry = lookupNode(graph,edge->target_id);
			if(person_entry != NULL)
			{
				person_entry = copy_entry_person_knows_person(trust_graph,person_entry);
			}

			LL_iter_next(iter);
		}
		LL_iter_destroy(iter);
	}


	////after insert and create trust graph

	HT_iter_ptr iterHT;
	ht_ptr nodes = Graph_nodes(trust_graph);
	ptr_entry data_trust;
	int graph_size = Graph_size(trust_graph);
	int j;
	list_ptr list_person;
	int size_person_list;

	ptr_entry new_one;
	ptr_edge trust_edge;

	double trust_apotel;
	ptr_entry node_i;
	ptr_entry node_j;

	iterHT = HT_iter_create(nodes);
	//printf("In getTopStalkers\n");

	for(i=0;i<graph_size;i++)
	{
		data_trust = ((ptr_entry)HT_iter_data(iterHT));
		//printf("Data id = %d\n",data_trust->id);
		list_person = type_list(data_trust,"person_knows_person.csv");
		if(list_person != NULL)
		{
			iter = LL_iter_create(list_person);
			size_person_list = LL_size(list_person);
			for(j=0;j<size_person_list;j++)
			{
				trust_edge = LL_iter_data(iter);
				new_one = lookupNode(trust_graph,trust_edge->target_id);
				if(new_one != NULL)
				{
					node_i = lookupNode(graph,data_trust->id);
					node_j = lookupNode(graph,new_one->id);

					//printf("Data id = %d with new one = %d\n",data_trust->id,new_one->id);
					trust_apotel = trust(node_i,node_j,graph,post_graph,comment_graph);
					edge_change_weight(trust_edge,trust_apotel);
				}

				LL_iter_next(iter);
			}
			LL_iter_destroy(iter);
		}

		HT_iter_next(iterHT);
	}

	HT_iter_destroy(iterHT);


	return trust_graph;
}



double estimateTrust(ptr_entry a, ptr_entry b, ptr_graph trust_graph)
{
	double trust_this = -1;

	list_ptr list_a = type_list(a,"person_knows_person.csv");
	LL_iter_ptr iter;
	int i;
	int list_size_a = 0;
	ptr_edge edge;
	list_ptr fringe = LL_create(match_trust);
	int flag = 0 ;//not found

	if(a->id == b->id)
	{
		return 1.0;
	}


	if(list_a != NULL)
	{
		//printf("ESTIM list_a != NULL\n");
		iter = LL_iter_create(list_a);
		list_size_a = LL_size(list_a);
		for(i=0;i<list_size_a;i++)
		{
			edge = ((ptr_edge) LL_iter_data(iter));
			//printf("edge id = %d weight = %f\n",edge->target_id,edge->weight);

			if((edge->weight != 0) && (edge->target_id != b->id))
			{
				//printf("edge id = %d\n",edge->target_id);
				check_to_insert_in_list(fringe,edge,1.0,0);
			}
			else if(edge->target_id == b->id)
			{
				trust_this = edge->weight;
				flag = 1;
				break;
			}

			LL_iter_next(iter);
		}
		LL_iter_destroy(iter);

	}

	int size_fringe = 0;
	size_fringe = LL_size(fringe);
	while(size_fringe != 0 && flag != 1)
	{
		if(flag == 0)
		{
			trust_this = rec_trust_search(trust_graph,fringe,b->id,&flag);
			size_fringe = LL_size(fringe);
		}
		else if(flag == 1)
		{
			break;
		}


	}

	return trust_this;


}


/************ new struct for estimate trust *******************/

struct data_trust
{
	int id;
	double trust_so_far;
	int level;
};


double data_trust_get_trust(ptr_data_trust data)
{
	return (data->trust_so_far);
}

int data_trust_get_id(ptr_data_trust data)
{
	return (data->id);
}

int data_trust_get_level(ptr_data_trust data)
{
	return (data->level);
}


int match_trust(const void *a, const void *key)
{
	if( ((ptr_data_trust)a)->id == *((int*)key) )
	{
		return 0;
	}
	else return 1;
}


ptr_data_trust create_data_trust(int id,double trust_so_far,int level)
{
	ptr_data_trust node;
	node = malloc(sizeof(struct data_trust));

	node->id = id;
	node->level = level;
	node->trust_so_far = trust_so_far;

	return node;
}

void destroy_data_trust(void* data)
{
	free(data);
}

int check_to_insert_in_list(list_ptr fringe,ptr_edge edge,double trust_so_far,int level)
{
	ptr_data_trust new_one,already_in_list,temp;
	double new_trust;
	int id_prev,level_prev;
	double trust_prev;
	//printf("^^^ check\n");

	already_in_list = ( (ptr_data_trust) LL_search(fringe,((void *) &(edge->target_id))) );
	if(already_in_list == NULL)
	{
		new_trust = trust_so_far * (edge->weight);
		//printf("*** Insert id = %d with weight = %f and trust = %f\n",edge->target_id,(edge->weight),new_trust);
		new_one = create_data_trust(edge->target_id,new_trust,level);
		LL_insert(fringe,((void *) new_one));
		return 1;    //inserted
	}
	else if(already_in_list != NULL)
	{
		id_prev = data_trust_get_id(already_in_list);
		level_prev = data_trust_get_level(already_in_list);
		trust_prev = data_trust_get_trust(already_in_list);

		if(level <= level_prev)
		{
			new_trust = trust_so_far * (edge->weight);
			if(new_trust > trust_prev)
			{
				//printf("*** Replace id = %d\n",edge->target_id);
				//printf("*** Insert id = %d with trust = %f\n",edge->target_id,new_trust);
				temp = already_in_list;
				new_one = create_data_trust(edge->target_id,new_trust,level);
				already_in_list = new_one;
				destroy_data_trust(((void *) temp));
				return 1;   //replaced
			}
			//printf("*** No Replace id = %d\n",edge->target_id);
		}
		//printf("*** No Replace id = %d\n",edge->target_id);

	}

	return 0; //nothing
}


//////////////////////////////////////////////////////////////////////////


double rec_trust_search(ptr_graph trust_graph,list_ptr fringe,int target_id,int *flag)
{
	ptr_data_trust first_data;
	ptr_data_trust new_one,already_in_list;
	int id_first, level;
	double trust_so_far,new_trust = 0.0;

	ptr_entry node;
	list_ptr list_node;
	ptr_edge edge;
	LL_iter_ptr iter;
	int i;
	int list_size_node = 0;

	first_data = LL_pop(fringe);
	id_first = data_trust_get_id(first_data);
	level = data_trust_get_level(first_data);
	trust_so_far = data_trust_get_trust(first_data);
	//printf("--- Del id = %d\n",id_first);

	node = lookupNode(trust_graph,id_first);
	if(node != NULL)
	{
		list_node = type_list(node,"person_knows_person.csv");
		if(list_node != NULL)
		{
			iter = LL_iter_create(list_node);
			list_size_node = LL_size(list_node);
			for(i=0;i<list_size_node;i++)
			{
				edge = ((ptr_edge) LL_iter_data(iter));

				if(edge->weight != 0 && edge->target_id != target_id)
				{
					level++;
					check_to_insert_in_list(fringe,edge,trust_so_far,level);
				}
				else if(edge->target_id == target_id)
				{
					new_trust = trust_so_far * (edge->weight);
					*flag = 1; //found target
					destroy_data_trust(((void *) first_data));
					return new_trust;
				}

				LL_iter_next(iter);

			}
			LL_iter_destroy(iter);
		}
	}


	destroy_data_trust(((void *) first_data));
	return new_trust;

}


