import java.util.Random;

public class PageReferenceString
{
  public static void main( String[] args )
  {
    int references = 1000;
    int max = 9;
    double hit = 0.73;
    Random r = new Random();
    int x = 1 + r.nextInt( max );  // [1,max]
    for ( int i = 0 ; i < references ; i++ )
    {
      if ( Math.random() > hit )
        x = 1 + r.nextInt( max );  // [1,max]
      System.out.print( x + " " );
    }
    System.out.println();
  }
}
