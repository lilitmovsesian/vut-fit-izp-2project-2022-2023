/*Lilit Movsesian*/
/*xmovse00*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <limits.h>
#include <string.h> /*This header was added to perform strlen() function in main.*/

#ifdef NDEBUG
#define debug(s)
#define dfmt(s, ...)
#define dint(i)
#define dfloat(f)
#else

#define debug(s) printf("- %s\n", s)

#define dfmt(s, ...) printf(" - "__FILE__":%u: "s"\n",__LINE__,__VA_ARGS__)

#define dint(i) printf(" - " __FILE__ ":%u: " #i " = %d\n", __LINE__, i)

#define dfloat(f) printf(" - " __FILE__ ":%u: " #f " = %g\n", __LINE__, f)

#endif

struct obj_t {
	int id;
	float x;
	float y;
};

struct cluster_t {
	int size;
	int capacity;
	struct obj_t *obj;
};

void init_cluster(struct cluster_t *c, int cap);
void clear_cluster(struct cluster_t *c);
struct cluster_t *resize_cluster(struct cluster_t *c, int new_cap);
void append_cluster(struct cluster_t *c, struct obj_t obj);
void sort_cluster(struct cluster_t *c);
void merge_clusters(struct cluster_t *c1, struct cluster_t *c2);
int remove_cluster(struct cluster_t *carr, int narr, int idx);
float obj_distance(struct obj_t *o1, struct obj_t *o2);
float cluster_distance(struct cluster_t *c1, struct cluster_t *c2);
void find_neighbours(struct cluster_t *carr, int narr, int *c1, int *c2);
static int obj_sort_compar(const void *a, const void *b);
void sort_cluster(struct cluster_t *c);
void print_cluster(struct cluster_t *c);
int load_clusters(char *filename, struct cluster_t **arr);
void print_clusters(struct cluster_t *carr, int narr);
void free_all (struct cluster_t *carr, int narr);
int is_valid_arg (char arg[], int len);


/*This functions allocates the memory for objects in clusters.*/
void init_cluster(struct cluster_t *c, int cap)
{
	assert(c != NULL);
	assert(cap >= 0);
	size_t size = sizeof(struct obj_t) * cap;
	void *arr = malloc(size);
	c->obj = (struct obj_t*)arr;
}

/*This function frees the memory allocated for objects in clusters.*/
void clear_cluster(struct cluster_t *c)
{
	free(c->obj);
}

const int CLUSTER_CHUNK = 10;

struct cluster_t *resize_cluster(struct cluster_t *c, int new_cap)
{
	assert(c);
	assert(c->capacity >= 0);
	assert(new_cap >= 0);

	if (c->capacity >= new_cap)
		return c;
	size_t size = sizeof(struct obj_t) * new_cap;
	void *arr = realloc(c->obj, size);
	if (arr == NULL)
		return NULL;
	c->obj = (struct obj_t*)arr;
	c->capacity = new_cap;
	return c;
}


/*Thifs function adds an object to a cluster and resizes it by one.*/
void append_cluster(struct cluster_t *c, struct obj_t obj)
{
	c = resize_cluster(c, c->capacity+1);
	c->size = c->size+1;
	c->obj[c->capacity-1] = obj;
}

/*This function merges two clusters by adding all objects of
second cluster to the first one. It deletes all objects from
second cluster and sorts the objects in newly merged cluster.*/
void merge_clusters(struct cluster_t *c1, struct cluster_t *c2)
{
	assert(c1 != NULL);
	assert(c2 != NULL);
	for (int i=0; i<c2->size; i++)
	{
		append_cluster(c1, c2->obj[i]);
	}
	clear_cluster(c2);
	sort_cluster(c1);
}

/*This function shifts each cluster in the array of clusters by one
index and returns new number of clusters in array.*/
int remove_cluster(struct cluster_t *carr, int narr, int idx)
{
	assert(idx < narr);
	assert(narr > 0);
	for (int i=idx; i<narr-1; i++)
	{
		carr[i]=carr[i+1];
	}
	return narr-1;
}

/*This functions counts the Euclidean distance of two objects.*/
float obj_distance(struct obj_t *o1, struct obj_t *o2)
{
	assert(o1 != NULL);
	assert(o2 != NULL);
	float res = sqrtf ((o1->x-o2->x)*(o1->x-o2->x)+(o1->y-o2->y)*(o1->y-o2->y));
	return res;
}

/*This functions compares each object of two clusters and returns the minimal
distance.*/
float cluster_distance(struct cluster_t *c1, struct cluster_t *c2)
{
	assert(c1 != NULL);
	assert(c1->size > 0);
	assert(c2 != NULL);
	assert(c2->size > 0);
	float res;
	float min_distance = obj_distance(&c1->obj[0], &c2->obj[0]);
	for (int i = 0; i<c1->size; i++)
	{
		for (int j=0; j<c2->size; j++)
		{
			res = obj_distance(&c1->obj[i], &c2->obj[j]);
			if (res < min_distance)
				min_distance = res;
		}
	}
	return min_distance;
}

/*This functions finds two nearest clusters and assign their index numbers
to the pointers from main.*/
void find_neighbours(struct cluster_t *carr, int narr, int *c1, int *c2)
{
	assert(narr > 0);
	float res;
	float min = cluster_distance(&carr[0], &carr[1]);
	for (int i=0; i<narr; i++)
	{
		for (int j=i+1; j<narr; j++)
		{
			res = cluster_distance(&carr[i], &carr[j]);
			if (res <= min)
			{
				min = res;
				*c1=i;
				*c2=j;
			}
		}
	}
}

static int obj_sort_compar(const void *a, const void *b)
{
	const struct obj_t *o1 = (const struct obj_t *)a;
	const struct obj_t *o2 = (const struct obj_t *)b;
	if (o1->id < o2->id)
		return -1;
	if (o1->id > o2->id)
		return 1;
	return 0;
}

void sort_cluster(struct cluster_t *c)
{
	qsort(c->obj, c->size, sizeof(struct obj_t), &obj_sort_compar);
}

void print_cluster(struct cluster_t *c)
{
	for (int i = 0; i < c->size; i++)
	{
		if (i) putchar(' ');
		printf("%d[%g,%g]", c->obj[i].id, c->obj[i].x, c->obj[i].y);
	}
	putchar('\n');
}

/*This function allocates memory for an array of struct cluster_t of
particular size. The data is read from a file and written to an
allocated array. The function also examines possible errors and
returns number of clusters.*/
int load_clusters(char *filename, struct cluster_t **arr)
{
	assert(arr != NULL);
	int i = 0, count;
	FILE *file = fopen (filename, "r");
	if (file == NULL)
	{
		fprintf (stderr, "File is NULL.\n");
		return -1;
	}
	fscanf(file, "count=%d[^\n]", &count);
	struct cluster_t *clustersarr;
	clustersarr = malloc(sizeof(struct cluster_t)*count);
 	if (clustersarr == NULL)
	{
		fprintf (stderr, "Memory allocation error.");
		return -1;
	}
	int id; float x, y;
	while (!feof(file) && i<count)
	{
		if (fscanf(file, "%d%f%f", &id, &x, &y) != 3)
		{
			fprintf (stderr, "Invalid clusters.\n");
			return -1;
		}
		clustersarr[i].size = 1;
		clustersarr[i].capacity = 1;
		init_cluster(&clustersarr[i],clustersarr[i].capacity);
		clustersarr[i].obj[0].id = id;
		clustersarr[i].obj[0].x = x;
		clustersarr[i].obj[0].y = y;
		i++;
	}
	*arr = clustersarr;
	fclose(file);
	return count;
}

void print_clusters(struct cluster_t *carr, int narr)
{
	printf("Clusters:\n");
	for (int i = 0; i < narr; i++)
	{
		printf("cluster %d: ", i);
		print_cluster(&carr[i]);
	}
}

/*This extra function was added to free all allocated memory.*/
void free_all (struct cluster_t *carr, int narr)
{
	for (int i=0; i<narr; i++)
	{
		clear_cluster(&carr[i]);
	}
	free(carr);
}

/*This extra function was added to examine the argv[2] characters.*/
int is_valid_arg (char arg[], int len)
{
	for (int i=0; i<len; i++)
	{
		if (arg[i] < '0' || arg[i] > '9')
			return 1;
	}
	return 0;
}

/*The possible input arguments are examined in the main function,
the program is terminated in case of invalid arguments. The
clustering is carried out in while loop, when the aim number
of clusters is reached, clusters are printed to stdout and
all memory is freed.*/
int main(int argc, char *argv[])
{
	int final_number;
	struct cluster_t *clusters;
	int countclusters = load_clusters (argv[1], &clusters);
	if (countclusters != -1 && argc == 2)
	{
		final_number = 1;
	}
	else if (countclusters != -1 && argc == 3 && atoi(argv[2])>0 && atoi(argv[2])<=countclusters && !is_valid_arg(argv[2], strlen(argv[2])))
	{
		final_number = atoi(argv[2]);
	}
	 else
	{
		fprintf (stderr, "Invalid number of arguments or invalid argument.\n");
		if (countclusters != -1)
		{
			free_all (clusters, countclusters);
		}
		return -1;
	}
	int index1, index2;
	while (countclusters != final_number)
	{
		find_neighbours(clusters, countclusters, &index1, &index2);
		merge_clusters(&clusters[index1], &clusters[index2]);
		countclusters=remove_cluster(clusters, countclusters, index2);
	}
	print_clusters(clusters, countclusters);
	free_all (clusters, countclusters);
	return 0;
}
