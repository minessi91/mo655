import java.util.Scanner;

public class ClassePrincipal {

	public static void main(String[] args) {
		
		Scanner teclado = new Scanner(System.in);
		System.out.println("Digite um valor ");
		int valor = teclado.nextInt();
		
		if (valor % 2 == 0) {
			System.out.println("O número "+valor+" é par!");
		} else {
			System.out.println("O número "+valor+" é impar");
		}
		
		
	}

}
