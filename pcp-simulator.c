#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

typedef struct book
{
	char *name;
} Book;

typedef struct pbThreadArgs
{
	int id;
	int pbType;
	int pbTypeAmount;
	int pbThreadAmount;
	int *bufferSizes;
	int bookLimit;
	int *booksPublishedPbType;
	int **booksPublishedPbThread;
	int *remainingPbThreads;
	pthread_mutex_t *generalMutexes;
	Book **buffers;
	sem_t *semaphores;
} PbThreadArgs;

typedef struct pcThreadArgs
{
	int id;
	int packageSize;
	int *packageSizes;
	int pbTypeAmount;
	int pbThreadAmount;
	int bookLimit;
	int *bufferSizes;
	int *remainingPbThreads;
	Book **buffers;
	Book **packages;
	pthread_mutex_t *generalMutexes;
	pthread_mutex_t *pcMutexes;
	sem_t *semaphores;
} PcThreadArgs;

// Prototypes
void *publish(void *parameters);
void *package(void *parameters);

// Main function
int main(int argc, char *argv[])
{
	// Taking the input from command line
	int pbTypeAmount = (int)strtol(argv[2], NULL, 10);
	int pbThreadAmount = (int)strtol(argv[3], NULL, 10);
	int pcThreadAmount = (int)strtol(argv[4], NULL, 10);
	int bookLimit = (int)strtol(argv[6], NULL, 10);
	int packageSize = (int)strtol(argv[8], NULL, 10);
	int bufferSize = (int)strtol(argv[9], NULL, 10);

	int i, j, rc;

	// Array of buffers
	Book **buffers = malloc(pbTypeAmount * sizeof(Book *));

	// Initialize the array of buffers
	for (i = 0; i < pbTypeAmount; i++)
		buffers[i] = NULL;

	// Array of buffer sizes
	int *bufferSizes = (int *)calloc(pbTypeAmount, sizeof(int));

	// Initialize the array of buffer sizes
	for (i = 0; i < pbTypeAmount; i++)
		bufferSizes[i] = bufferSize;

	// Integer array of book amounts published for each publisher type
	int *booksPublishedPbType = (int *)calloc(pbTypeAmount, sizeof(int));

	// Integer double array of book amounts published for each publisher thread
	int **booksPublishedPbThread = malloc(pbTypeAmount * sizeof(int *));

	// Initialize the array of book amounts published for each publisher thread
	for (i = 0; i < pbTypeAmount; i++)
		booksPublishedPbThread[i] = (int *)calloc(pbThreadAmount, sizeof(int));

	// Array of generalMutexes for both publishers ans packagers
	pthread_mutex_t *generalMutexes = (pthread_mutex_t *)malloc(pbTypeAmount * sizeof(pthread_mutex_t));

	// Initialize the generalMutexes
	for (i = 0; i < pbTypeAmount; i++)
		pthread_mutex_init(&generalMutexes[i], NULL);

	// Double array of publisher threads
	pthread_t pbThreads[pbTypeAmount][pbThreadAmount];

	// Array of parameter structs for publisher threads
	PbThreadArgs *pbThreadParameters = (PbThreadArgs *)malloc(pbTypeAmount * pbThreadAmount * sizeof(PbThreadArgs));

	// Array of remaining publisher thread amounts
	int *remainingPbThreads = (int *)malloc(pbTypeAmount * sizeof(int));

	// Initialize remainingPbThreads
	for (i = 0; i < pbTypeAmount; i++)
		remainingPbThreads[i] = pbThreadAmount;

	// Array of semaphores
	sem_t *semaphores = (sem_t *)malloc(pbTypeAmount * sizeof(sem_t));

	// Initialize semaphores
	for (i = 0; i < pbTypeAmount; i++)
		sem_init(&semaphores[i], 0, 0);

	// Creating the publisher threads
	for (i = 0; i < pbTypeAmount; i++)
	{
		for (j = 0; j < pbThreadAmount; j++)
		{
			pbThreadParameters[i * pbThreadAmount + j].id = j;
			pbThreadParameters[i * pbThreadAmount + j].pbType = i;
			pbThreadParameters[i * pbThreadAmount + j].pbTypeAmount = pbTypeAmount;
			pbThreadParameters[i * pbThreadAmount + j].pbThreadAmount = pbThreadAmount;
			pbThreadParameters[i * pbThreadAmount + j].bufferSizes = bufferSizes;
			pbThreadParameters[i * pbThreadAmount + j].bookLimit = bookLimit;
			pbThreadParameters[i * pbThreadAmount + j].booksPublishedPbType = booksPublishedPbType;
			pbThreadParameters[i * pbThreadAmount + j].booksPublishedPbThread = booksPublishedPbThread;
			pbThreadParameters[i * pbThreadAmount + j].remainingPbThreads = remainingPbThreads;
			pbThreadParameters[i * pbThreadAmount + j].generalMutexes = generalMutexes;
			pbThreadParameters[i * pbThreadAmount + j].buffers = buffers;
			pbThreadParameters[i * pbThreadAmount + j].semaphores = semaphores;

			rc = pthread_create(&pbThreads[i][j], NULL, publish, (void *)&pbThreadParameters[i * pbThreadAmount + j]);
			if (rc)
			{
				/* ***Log Message*** */
				printf("ERROR: The thread couldn't be created succesfully. Return code: %d\n", rc);
				exit(1);
			}
		}
	}

	// Array of packager threads
	pthread_t pcThreads[pcThreadAmount];

	// Array of parameter structs for packager threads
	PcThreadArgs *pcThreadParameters = (PcThreadArgs *)malloc(pcThreadAmount * sizeof(PcThreadArgs));

	// Array of mutexes for packagers
	pthread_mutex_t *pcMutexes = (pthread_mutex_t *)malloc(pbTypeAmount * sizeof(pthread_mutex_t));

	// Initialize pcMutexes
	for (int i = 0; i < pbTypeAmount; i++)
		pthread_mutex_init(&pcMutexes[i], NULL);

	// Array of packages
	Book **packages = malloc(pcThreadAmount * sizeof(Book *));

	// Initilaize packages
	for (i = 0; i < pcThreadAmount; i++)
		packages[i] = NULL;

	// Array of package sizes
	int *packageSizes = (int *)calloc(pcThreadAmount, sizeof(int));

	// Initialize the packageSizes
	for (i = 0; i < pcThreadAmount; i++)
		packageSizes[i] = packageSize;

	// Creating the packager threads
	for (i = 0; i < pcThreadAmount; i++)
	{
		pcThreadParameters[i].id = i;
		pcThreadParameters[i].packageSize = packageSize;
		pcThreadParameters[i].packageSizes = packageSizes;
		pcThreadParameters[i].pbTypeAmount = pbTypeAmount;
		pcThreadParameters[i].pbThreadAmount = pbThreadAmount;
		pcThreadParameters[i].bookLimit = bookLimit;
		pcThreadParameters[i].remainingPbThreads = remainingPbThreads;
		pcThreadParameters[i].bufferSizes = bufferSizes;
		pcThreadParameters[i].buffers = buffers;
		pcThreadParameters[i].generalMutexes = generalMutexes;
		pcThreadParameters[i].pcMutexes = pcMutexes;
		pcThreadParameters[i].packages = packages;
		pcThreadParameters[i].semaphores = semaphores;

		rc = pthread_create(&pcThreads[i], NULL, package, (void *)&pcThreadParameters[i]);
		if (rc)
		{
			/* ***Log Message*** */
			printf("ERROR: The thread couldn't be created succesfully. Return code: %d\n", rc);
			exit(1);
		}
	}

	pthread_exit(NULL);
}

void *publish(void *parameters)
{
	PbThreadArgs *args = (PbThreadArgs *)parameters;

	int id = args->id;
	int pbType = args->pbType;
	int pbTypeAmount = args->pbTypeAmount;
	int pbThreadAmount = args->pbThreadAmount;
	int *bufferSizes = args->bufferSizes;
	int bookLimit = args->bookLimit;
	int *booksPublishedPbType = args->booksPublishedPbType;
	int **booksPublishedPbThread = args->booksPublishedPbThread;
	int *remainingPbThreads = args->remainingPbThreads;
	pthread_mutex_t *generalMutexes = args->generalMutexes;
	Book **buffers = args->buffers;
	sem_t *semaphores = args->semaphores;

	while (1)
	{
		// Lock the mutex for the buffer of publisher type pbType
		pthread_mutex_lock(&generalMutexes[pbType]);

		int i;
		// If the buffer of the publisher type doesn't exist, create it
		if (buffers[pbType] == NULL)
		{
			Book *buffer = (Book *)malloc(bufferSizes[pbType] * sizeof(Book));
			for (i = 0; i < bufferSizes[pbType]; i++)
			{
				Book book;
				book.name = "empty";
				buffer[i] = book;
			}
			buffers[pbType] = buffer;
		}

		// Create a new book
		Book book;

		// Filling book.name
		char array[100] = "";
		char *x = "Book";
		char *y = "_";

		int a = pbType + 1;
		int b = booksPublishedPbType[pbType] + 1;
		char z[10] = "";
		char w[10] = "";
		sprintf(z, "%d", a);
		sprintf(w, "%d", b);

		strcat(array, x);
		strcat(array, z);
		strcat(array, y);
		strcat(array, w);
		book.name = array;

		int emptyCount = 0;
		char *emptyName = "empty";

		// Counting the empty places in the buffer
		for (i = 0; i < bufferSizes[pbType]; i++)
		{
			if (strncmp(buffers[pbType][i].name, emptyName, 5) == 0)
				emptyCount++;
		}

		// These variables are for output purposes
		int idIncremented = id + 1;
		int pbTypeIncremented = pbType + 1;

		// If there's no empty space in the buffer, create a new one
		if (emptyCount == 0)
		{
			/* ***Log Message*** */
			printf("Publisher %d of type %d:          Buffer is full. Resizing the buffer.\n",
				   idIncremented, pbTypeIncremented);

			// Create the new expanded buffer
			Book *buffer = (Book *)malloc(2 * bufferSizes[pbType] * sizeof(Book));

			// Put the values inside the old buffer into the new expanded buffer
			for (i = 0; i < bufferSizes[pbType]; i++)
				buffer[i] = buffers[pbType][i];

			// Put the books with names "empty" to the newly created empty spaces
			for (i = bufferSizes[pbType]; i < 2 * bufferSizes[pbType]; i++)
			{
				Book book1;
				book1.name = "empty";
				buffer[i] = book1;
			}

			free(buffers[pbType]);
			buffers[pbType] = buffer;
			bufferSizes[pbType] = 2 * bufferSizes[pbType];
		}

		// Inserting the book created above to the buffer
		for (i = 0; i < bufferSizes[pbType]; i++)
		{
			char *emptyName = "empty";
			if (strncmp(buffers[pbType][i].name, emptyName, 5) == 0)
			{
				/* ***Log Message*** */
				printf("Publisher %d of type %d:          %s is published and put into the buffer %d.\n",
					   idIncremented, pbTypeIncremented, book.name, pbTypeIncremented);

				buffers[pbType][i] = book;
				printf("book name: %s\n", buffers[pbType][i].name);
				booksPublishedPbType[pbType]++;
				booksPublishedPbThread[pbType][id]++;

				// Increment the semaphore
				if (emptyCount == 0)
					sem_post(&semaphores[pbType]);

				if (booksPublishedPbThread[pbType][id] == bookLimit)
				{
					/* ***Log Message*** */
					printf("Publisher %d of type %d:          Finished publishing %d books. Exiting the system.\n",
						   idIncremented, pbTypeIncremented, bookLimit);
					remainingPbThreads[pbType]--;
					pthread_mutex_unlock(&generalMutexes[pbType]);
					pthread_exit(NULL);
				}

				break;
			}
		}

		// Unlock the mutex
		pthread_mutex_unlock(&generalMutexes[pbType]);
		sleep(1);
	}
}

void *package(void *parameters)
{
	PcThreadArgs *args = (PcThreadArgs *)parameters;

	int id = args->id;
	int packageSize = args->packageSize;
	int *packageSizes = args->packageSizes;
	int pbTypeAmount = args->pbTypeAmount;
	int pbThreadAmount = args->pbThreadAmount;
	int bookLimit = args->bookLimit;
	int *bufferSizes = args->bufferSizes;
	int *remainingPbThreads = args->remainingPbThreads;
	Book **buffers = args->buffers;
	Book **packages = args->packages;
	pthread_mutex_t *generalMutexes = args->generalMutexes;
	pthread_mutex_t *pcMutexes = args->pcMutexes;
	sem_t *semaphores = args->semaphores;

	while (1)
	{
		// This is for output purposes
		int idIncremented = id + 1;

		int finishedPbTypeAmount = 0;
		int *selected = (int *)calloc(pbTypeAmount, sizeof(int));
		int pbType;

		while (1)
		{
			//If all publisher threads left the system, packager thread will exit
			if (finishedPbTypeAmount == pbTypeAmount)
			{
				int packagedBooks = 0;
				int allBooks = pbTypeAmount * pbThreadAmount * bookLimit;

				int i;
				char *emptyName = "empty";

				// Count the packaged books for this packager thread
				for (i = 0; i < packageSizes[id]; i++)
				{
					if (strncmp(packages[id][i].name, emptyName, 5) == 0)
						break;

					packagedBooks++;
				}

				/* ***Log Message*** */
				printf("Packager %d:                     All publisher threads have left the system. %d of %d books have been packaged.\n",
					   idIncremented, packagedBooks, allBooks);

				int packageAmount = packageSizes[id] / packageSize;
				int j = packageAmount;

				for (i = 0; i < packageAmount; i++)
				{
					char array[100] = "";
					char *x = "Package ";

					int a = i + 1;
					char y[10] = "";
					sprintf(y, "%d", a);

					strcat(array, x);
					strcat(array, y);
					char *packageName = array;

					printf("     %s: ", packageName);
					int k;
					for (k = packageSizes[id] - j * packageSize; k < packageSizes[id] - (j - 1) * packageSize; k++)
					{
						if (k == packageSizes[id] - (j - 1) * packageSize - 1)
							printf("%s", packages[id][k].name);
						else
							printf("%s, ", packages[id][k].name);
					}
					printf("\n");
				}

				pthread_exit(NULL);
			}

			while (1)
			{
				//Select a random buffer from buffers
				srand(time(NULL));
				pbType = rand() % pbTypeAmount;

				// Lock the corresponding buffer for other packagers
				pthread_mutex_lock(&pcMutexes[pbType]);

				// Check if the buffer is created
				if (buffers[pbType] == NULL)
					continue;

				// Check if selected buffer is empty, and if there are any threads left for that buffer
				char *emptyName = "empty";
				if (strncmp(buffers[pbType][0].name, emptyName, 5) == 0)
				{
					if (remainingPbThreads[pbType] == 0)
					{
						if (selected[pbType] == 1)
						{
							pthread_mutex_unlock(&pcMutexes[pbType]);
							continue;
						}
						else
						{
							finishedPbTypeAmount++;
							selected[pbType] = 1;
							pthread_mutex_unlock(&pcMutexes[pbType]);
											
							if (finishedPbTypeAmount == pbTypeAmount)
								break;

							continue;
						}
					}
					else
					{
						// Decrement the semaphore
						sem_wait(&semaphores[pbType]);
						break;
					}
				}

				break;
			}

			if (finishedPbTypeAmount == pbTypeAmount)
				continue;

			break;
		}

		// Lock the corresponding buffer for everyone
		pthread_mutex_lock(&generalMutexes[pbType]);

		int i;
		// If a package doesn't exist, create it
		if (packages[id] == NULL)
		{
			Book *package = (Book *)malloc(packageSizes[id] * sizeof(Book));
			for (i = 0; i < packageSizes[id]; i++)
			{
				Book book;
				book.name = "empty";
				package[i] = book;
			}
			packages[id] = package;
		}

		// Add the oldest book in the corresponding buffer into the package
		for (i = 0; i < packageSizes[id]; i++)
		{
			int j;
			char *emptyName = "empty";
			if (strncmp(packages[id][i].name, emptyName, 5) != 0)
				continue;

			/* ***Log Message*** */
			printf("Packager %d:                     Put %s into the package.\n",
				   idIncremented, buffers[pbType][0].name);

			packages[id][i] = buffers[pbType][0];

			// If packages[id] is full after the insertion, expand the packages[id] for new packages
			if (i == packageSizes[id] - 1)
			{
				/* ***Log Message*** */
				printf("Packager %d:                     Finished preparing one package. The package contains: ",
					   idIncremented);

				for (j = packageSizes[id] - packageSize; j < packageSizes[id]; j++)
				{
					if (j == packageSizes[id] - 1)
						printf("%s", packages[id][j].name);
					else
						printf("%s, ", packages[id][j].name);
				}
				printf("\n");

				// Create the new expanded package array
				Book *package = (Book *)malloc((packageSizes[id] + packageSize) * sizeof(Book));

				// Put the values inside the old package array into the new expanded package array
				for (j = 0; j < packageSizes[id]; j++)
					package[j] = packages[id][j];

				// Put the books named "empty" to the newly created empty spaces
				for (j = packageSizes[id]; j < (packageSizes[id] + packageSize); j++)
				{
					Book book;
					book.name = "empty";
					package[j] = book;
				}

				//free(packages[id]);
				packages[id] = package;
				packageSizes[id] = packageSizes[id] + packageSize;
			}

			// Rearrange the elements of the corresponding buffer (shift to left)
			int x;
			printf("Buffer%d, Before packaging:", pbType + 1);
			for (x = 0; x < bufferSizes[pbType]; x++)
				printf("%s ", buffers[pbType][x].name);
			printf("\n");

			j = 1;
			while (strncmp(buffers[pbType][j].name, emptyName, 5) != 0)
			{
				buffers[pbType][j - 1] = buffers[pbType][j];
				j++;
			}
			Book book;
			book.name = "empty";
			buffers[pbType][j - 1] = book;

			printf("Buffer%d, After packaging:", pbType + 1);
			for (x = 0; x < bufferSizes[pbType]; x++)
				printf("%s ", buffers[pbType][x].name);
			printf("\n");		

			break;
		}

		// Unlock all the mutexes
		pthread_mutex_unlock(&pcMutexes[pbType]);
		pthread_mutex_unlock(&generalMutexes[pbType]);
		sleep(1);
	}
}
