func createButton: any -> {
	click: func {
		console << "Clicked !\n";
	}
}

func main {
	var button = createButton();
	button.click();
}
