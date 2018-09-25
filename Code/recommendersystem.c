#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<math.h>

double** ComputeCoefficient(int m,int n,double **Mean);

int main()
{
    int m=943,n=1682;
    FILE *fd;
    int size=0,i,j,k;
    int **matrix;
    char record[100];
    char **records_array;

    fd = fopen("train_all_txt.txt","r");
    if(!fd)
    {
        printf("Unable to open file train_all_txt.txt\n");
        exit(1);
    }
	
    //Read and process records
    int record_size=0;
    records_array = malloc(1000000*sizeof(char*));
    while(fgets(record,100,fd)!= NULL)
    {
        int len=0;
        len = strlen(record);

        if(record[len-1] == '\n')
        {
            char *temp;
            temp = strtok(record,"\n");
            records_array[record_size] = malloc(strlen(temp)*sizeof(char));
            memmove(records_array[record_size],temp,strlen(temp));
        }
        else
        {
            records_array[record_size] = malloc(strlen(record)*sizeof(char));
            memmove(records_array[record_size],record,strlen(record));
        }
        record_size++;
    }
	
        //Create Matrix
        size=record_size;
        matrix = (int **)malloc(size*sizeof(int *));
        for (i=0; i<size; i++)
        {
            matrix[i] = malloc(size*sizeof(int));
        }

        //Store records in matrix
        i=0;
	for(i=0;i<size;i++)
	{
		char *temp;
		temp = strtok(records_array[i]," ");
		j=0;
		matrix[i][j++] = atoi(temp);
		while(j<3)
   		{
			temp = strtok(NULL," ");
			matrix[i][j++] = atoi(temp);	
		}
	}

        //Initialize a new matrix
        int new_matrix[m+1][n+1];
        for(i=0;i<m+1;i++)
        {   
            for(j=0;j<n+1;j++)
            {
                new_matrix[i][j] = 0;
            } 
        }

        //Copy currently present user,item and rating details to new matrix
        i = 0;
	for(i=0;i<size;i++)
	{
            int new_i=0,new_j=0;
            new_i = matrix[i][0];
            new_j = matrix[i][1];
            new_matrix[new_i][new_j] = matrix[i][2];
        }

        //Compute average rating per user
        double avgRatingPerUser[m+1];
        for(i=1;i<m+1;i++)
	{
                double avg=0.0;
		for(j=1;j<n+1;j++)
                {
                    avg+=new_matrix[i][j]; 
		}
		avgRatingPerUser[i] = avg/n;
        }

        //Create a matrix to save ratings minus avg ratings
        double **Mean;
        Mean = malloc((n+1)*sizeof(double*));
        for(i = 0;i<n+1;i++) {
        Mean[i] = malloc((n+1)*sizeof(double));
        }

        //Compute current rating of user for a item minus avg rating of that user
        for(i=1;i<m+1;i++)
	{
            for(j=1;j<n+1;j++)
            {
                Mean[i][j] = new_matrix[i][j] - avgRatingPerUser[i];
            }
        }

    //Create a coefficient matrix
    double **Coefficient;
    Coefficient = malloc((n+1)*sizeof(double*));
    for(i = 0;i<n+1;i++) {
    Coefficient[i] = malloc((n+1)*sizeof(double));
    }

    //Function call to compute Pearson's Correlation Coefficient
    Coefficient = ComputeCoefficient(n,n,Mean);

    //Create a matrix that will contain predicted ratings
    double **PredictedRatings;
    PredictedRatings = malloc((m+1)*sizeof(double*));
    for(i = 0; i < m+1; i++) {
        PredictedRatings[i] = malloc((m+1)*sizeof(double));
    }

    //Create and open output.txt file in write mode
    FILE *fpw;
    fpw = fopen("output.txt","w");
    if(!fpw)
    {
        printf("Unable to open file output.txt\n");
        exit(1);
    }

    double numerator = 0,denominator = 0,rating = 0;
    //Logic to predict missing ratings
    for(i=1;i<m+1;i++)
    {
	for(j=1;j<n+1;j++)
	{
            //If rating was already present,add it to PredictedRating matrix and output file
            if(new_matrix[i][j] != 0)
            {
                PredictedRatings[i][j]=new_matrix[i][j];
                //Save matrix to file
                if(i==m && j==n)
                {
                    fprintf(fpw,"%d % d %d",i,j,(int)PredictedRatings[i][j]);
                }
                else
                {
                    fprintf(fpw,"%d % d %d\n",i,j,(int)PredictedRatings[i][j]);
                }
            }
            else //Predict rating if it was zero in input matrix
            {

                //Compute numerator and denominator
		for(k=1;k<m+1;k++)
		{
                    if(new_matrix[k][j] !=0)
                    {
                        if(i!=k)
                        {
                            numerator = numerator + (new_matrix[k][j]*Coefficient[i][k]);
                            denominator = denominator + fabs(Coefficient[i][k]);
                        }
                    }	
		}
		
                //Predicted rating
                rating = numerator/denominator;
		
                //Assign ratings as per prediction and ratings range
                if(rating < 1)
                    rating = 1;
		else if(rating > 5)
                    rating = 5;
                else if(rating >= 1 || rating <= 5){
                    rating = rating;
                }
                else
                    rating = 1;
                         
                //Round-off and type-cast predicted rating
		PredictedRatings[i][j]= (int)round(rating);

                //Save matrix to file
                if(i==m && j==n)
                {
                    fprintf(fpw,"%d % d %d",i,j,(int)PredictedRatings[i][j]);
                }
                else
                {
                    fprintf(fpw,"%d % d %d\n",i,j,(int)PredictedRatings[i][j]);
                }
                numerator = 0,denominator = 0,rating = 0;
            } 
	}
    }

    fclose(fpw);

    return 0;
}

//Function to Compute Pearson's Correlation Coefficient
double** ComputeCoefficient(int m,int n,double **Mean)
{
    int i,j,k;
    double numerator = 0,denominator = 0,x = 0,y = 0,xy = 0;
    double **Coefficient;

    //Allocate memory to coefficient matrix
    Coefficient = malloc((n+1)*sizeof(double*));
    for(i = 0;i<n+1;i++)
    {
        Coefficient[i] = malloc((n+1)*sizeof(double));
    }
       
    //Compute Pearson's Correlation Coefficient
    for(i=1;i<m+1;i++)
    {
        for(j=1;j<m+1;j++)
        {
            if(i != j)
            {
                //Compute x,y and numerator for Pearson's algo
                for(k=1;k<n+1;k++)
                {
                    numerator += Mean[i][k] * Mean[j][k];
                    x +=  pow(Mean[i][k],2);
                    y += pow(Mean[j][k],2);
                }

                //Compute xy
                xy = x * y;

                //Compute square root of xy
                denominator = sqrt(xy);
	
                //Compute Pearson's Correlation Coefficient
                Coefficient[i][j] = numerator/denominator;
                numerator = 0,denominator = 0,x = 0,y = 0,xy = 0;
            }
            else
                continue;
        }
    }
    return Coefficient;
}
