from django import forms

class ContactForm(forms.Form):
    name = forms.CharField(label='', widget=forms.TextInput(attrs={'class':'col-lg-7', 'placeholder':'What are you looking for?','autocomplete': 'off'}))
    # email  = forms.EmailField(label='E-mail')
