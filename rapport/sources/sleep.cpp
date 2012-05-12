while(!this->has_token)
{
	// sleep for 1 second
	SDL_Delay(1000);
}	

this->critical_section();
